#include "VM/include/RigCVM/RigCVMPCH.hpp"

#include <RigCVM/Executors/All.hpp>
#include <RigCVM/Executors/ExpressionExecutor.hpp>
#include <RigCVM/Executors/Templates.hpp>
#include <RigCVM/VM.hpp>
#include <RigCVM/StackFrame.hpp>

#include <RigCVM/TypeSystem/ArrayType.hpp>
#include <RigCVM/TypeSystem/RefType.hpp>
#include <RigCVM/TypeSystem/ClassType.hpp>
#include <RigCVM/TypeSystem/UnionType.hpp>
#include <RigCVM/TypeSystem/EnumType.hpp>

namespace rigc::vm
{
#define MAKE_EXECUTOR(ClassName, Executor) { #ClassName, Executor }

Map<ExecutorTrigger, ExecutorFunction*, std::less<>> Executors = {
	MAKE_EXECUTOR(ImportStatement,					executeImportStatement),
	MAKE_EXECUTOR(CodeBlock,						executeCodeBlock),
	MAKE_EXECUTOR(IfStatement,						executeIfStatement),
	MAKE_EXECUTOR(WhileStatement,					executeWhileStatement),
	MAKE_EXECUTOR(ForStatement,						executeForStatement),
	MAKE_EXECUTOR(ReturnStatement,					executeReturnStatement),
	MAKE_EXECUTOR(SingleBlockStatement,				executeSingleStatement),
	MAKE_EXECUTOR(Expression,						evaluateExpression),
	MAKE_EXECUTOR(BreakStatement,					evaluateBreakStatement),
	MAKE_EXECUTOR(ContinueStatement,				evaluateContinueStatement),
	MAKE_EXECUTOR(PossiblyTemplatedSymbol,			evaluateSymbol),
	MAKE_EXECUTOR(PossiblyTemplatedSymbolNoDisamb,	evaluateSymbol),
	MAKE_EXECUTOR(Name,								evaluateName),
	MAKE_EXECUTOR(IntegerLiteral,					evaluateIntegerLiteral),
	MAKE_EXECUTOR(BoolLiteral,						evaluateBoolLiteral),
	MAKE_EXECUTOR(StringLiteral,					evaluateStringLiteral),
	MAKE_EXECUTOR(CharLiteral,						evaluateCharLiteral),
	MAKE_EXECUTOR(Float32Literal,					evaluateFloat32Literal),
	MAKE_EXECUTOR(Float64Literal,					evaluateFloat64Literal),
	// MAKE_EXECUTOR(ArrayLiteral,					evaluateArrayLiteral),
	// MAKE_EXECUTOR(ArrayElement,					evaluateArrayElement),
	MAKE_EXECUTOR(VariableDefinition,				evaluateVariableDefinition),
	MAKE_EXECUTOR(FunctionDefinition,				evaluateFunctionDefinition),
	MAKE_EXECUTOR(InitializerValue,					evaluateExpression),
	MAKE_EXECUTOR(FunctionArg,						evaluateExpression),
	MAKE_EXECUTOR(ClassDefinition,					evaluateClassDefinition),
	MAKE_EXECUTOR(EnumDefinition,					executeEnumDefinition),
	MAKE_EXECUTOR(MethodDef,						evaluateMethodDefinition),
	MAKE_EXECUTOR(MemberOperatorDef,				evaluateMemberOperatorDefinition),
	MAKE_EXECUTOR(DataMemberDef,					evaluateDataMemberDefinition),
};

#undef MAKE_EXECUTOR

////////////////////////////////////////
auto executeCodeBlock(Instance &vm_, rigc::ParserNode const& codeBlock_) -> OptValue
{
	auto scope = StackFramePusher(vm_, codeBlock_);

	auto stmts = findElem<rigc::Statements>(codeBlock_);

	if (stmts)
	{
		for (auto const& stmt : stmts->children)
		{
			auto stackFramePos = vm_.stack.size;
			OptValue val = vm_.evaluate(*stmt);
			// if (stmt->is_type<rigc::Expression>())
			// 	vm_.stack.size = stackFramePos;

			if (vm_.returnTriggered)
				return val;
			else if(vm_.continueTriggered)
				return {};
			else if(vm_.breakLevel)
				return {};
		}
	}

	return {};
}

////////////////////////////////////////
auto executeSingleStatement(Instance &vm_, rigc::ParserNode const& stmt_) -> OptValue
{
	auto scope = StackFramePusher(vm_, stmt_);

	for (auto const& childStmt : stmt_.children)
	{
		auto ret = vm_.evaluate(*childStmt);
		if (vm_.returnTriggered)
			return ret;
	}

	return {};
}

////////////////////////////////////////
auto evaluateExpression(Instance &vm_, rigc::ParserNode const& expr_) -> OptValue
{
	return ExpressionExecutor{vm_, expr_}.evaluate();
}

//todo refactor evaluateSymbol and evaluateName
////////////////////////////////////////
auto evaluateSymbol(Instance &vm_, rigc::ParserNode const& expr_) -> OptValue
{
	// Either "PossiblyTemplatedSymbol" or "PossiblyTemplatedSymbolNoDisamb"
	auto& name	= *findElem<rigc::Name>(expr_, false);

	if (name.string_view() == "null")
	{
		// TODO: protect against `null<TemplateArgs...>`
		return vm_.allocateOnStack<void const*>(vm_.builtinTypes.Null.shared(), nullptr);
	}

	auto opt	= vm_.findVariableByName(name.string_view());

	if (!opt) {
		auto func = vm_.findFunction(name.string_view());
		if (!func.empty())
		{
			// fmt::print("Found function: {}\n", name.string_view());
			// PLACEHOLDER:
			opt = vm_.functionValue(*func.front().second->front());
		}
	}

	if (!opt)
	{
		throw RigCError("Unrecognized identifier with name \"{}\"", name.string())
						.withHelp("Check the spelling of the identifier.")
						.withLine(vm_.lastEvaluatedLine);
	}

	if (opt->type->is<RefType>())
		return opt;

	return vm_.allocateReference(opt.value());
}

////////////////////////////////////////
auto evaluateName(Instance &vm_, rigc::ParserNode const& expr_) -> OptValue
{
	auto opt = vm_.findVariableByName(expr_.string_view());

	// if (!opt) {
	// 	opt = vm_.findFunctionExpr(expr_.string_view());
	// }

	if (!opt)
	{
		throw RigCError("Unrecognized identifier with name \"{}\"", expr_.string())
						.withHelp("Check the spelling of the identifier.")
						.withLine(vm_.lastEvaluatedLine);
	}

	if (auto ref = opt->type->as<RefType>())
		return opt;

	return vm_.allocateReference(opt.value());
}

bool copyConstructOn(Instance& vm_, Value constructed_, Value const& copyFrom_);

////////////////////////////////////////
auto evaluateVariableDefinition(Instance &vm_, rigc::ParserNode const& expr_) -> OptValue
{
	auto declType	= findElem<rigc::DeclType>(expr_, false)->string_view();
	auto varName	= findElem<rigc::Name>(expr_, false)->string_view();
	auto valueExpr	= findElem<rigc::InitializerValue>(expr_, false);

	bool deduceType = (declType == "var" || declType == "const");

	size_t currentStackSize = vm_.stack.size;

	Value value;
	if (valueExpr)
	{
		value = vm_.evaluate(*valueExpr).value();
		if (auto ref = value.type->as<RefType>())
		{
			value = value.removeRef();

			// If the value existed before the definition it means it is a reference
			// to another variable.
			// FIXME: This is a hack that will break eventually. We need to create an x-value type.
			if (static_cast<const char*>(value.data) - vm_.stack.data() < ptrdiff_t(currentStackSize))
			{
				auto newValue = vm_.allocateOnStack(value.type, nullptr, 0);
				if (!copyConstructOn(vm_, newValue, value))
				{
					// Byte-wise copy
					std::memcpy(newValue.data, value.data, value.type->size());
				}

				value = newValue;
			}
		}
	}
	else if (deduceType)
	{
		throw RigCError("Variable {} requires an initializer, because of type deduction using \"{}\"", varName, declType)
						.withHelp("Provide an initializer for the variable.")
						.withLine(vm_.lastEvaluatedLine);
	}

	DeclType type;
	if (deduceType)
		type = value.type;
	else
	{
		if (auto t = vm_.findType(declType))
			type = t->shared_from_this();
		else
			throw RigCError("Type {} not found", declType)
							.withHelp("Check the spelling of the type.")
							.withLine(vm_.lastEvaluatedLine);

		if (!valueExpr)
		{
			value = vm_.allocateOnStack(type, nullptr);

			if (auto c = type->as<ClassType>())
			{
				if (auto ctor = c->defaultConstructor())
				{
					Function::Args args;
					args[0] = vm_.allocateReference(value);
					vm_.executeFunction(*ctor, Function::ArgSpan{ args.data(), 1 } );
				}
			}
		}
		else if (value.type != type)
		{
			auto converted = vm_.tryConvert(value, type);
			if (!converted)
				throw RigCError("Cannot convert {} to {}", value.type->name(), type->name())
								.withLine(vm_.lastEvaluatedLine);

			value = converted.value();
		}
	}

	// value = vm_.cloneValue(value);

	if (!vm_.currentScope->variables.contains(varName))
	{
		auto varNameStr = String(varName);
		auto& var = vm_.currentScope->variables[varNameStr];
		var = FrameBasedValue::fromAbsolute(value, vm_.stack.frames.back());
	}

	return value;
}

////////////////////////////////////////
auto executeUnionDefinition(Instance &vm_, rigc::ParserNode const& expr_) -> OptValue
{
	// auto const templateParamList = getTemplateParamList(expr_);
	// std::pair<String, TypeConstraint>, string is a name,
	// TypeConstraint is, for now, a struct with just a name (String)
	// TODO: actually do something with the template parameter list

	auto type = std::make_shared<UnionType>();
	type->parse(expr_);
	vm_.currentScope->addType(type);

	auto prevClass = vm_.currentClass;
	vm_.currentClass = type.get();

	// Evaluate the class body
	auto body = findElem<rigc::UnionCodeBlock>(expr_, false);
	for (auto const& child : body->children)
	{
		vm_.evaluate(*child);
	}

	vm_.currentClass = prevClass;

	return {};
}

////////////////////////////////////////
auto executeEnumDefinition(Instance &vm_, rigc::ParserNode const& expr_) -> OptValue
{

	auto type = std::make_shared<EnumType>();
	type->parse(expr_);

	auto const typeExpr = findNthElem<rigc::Name>(expr_, 2);

	type->underlyingType = [&typeExpr, &vm_]() -> IType const* {
		if(!typeExpr) return vm_.builtinTypes.Int32.raw;

		auto const underlying = vm_.findType(typeExpr->string_view());
		if(!underlying)
			throw RigCError("Unknown type \"{}\" for enum.", typeExpr->string_view())
							.withHelp("Check the spelling of the type.")
							.withLine(vm_.lastEvaluatedLine);

		return underlying;
	}()->shared_from_this();

	vm_.currentScope->addType(type);

	auto prevClass = vm_.currentClass;
	vm_.currentClass = type.get();

	// Evaluate the class body
	auto body = findElem<rigc::EnumCodeBlock>(expr_, false);
	for (auto const& child : body->children)
	{
		vm_.evaluate(*child);
	}

	vm_.currentClass = prevClass;

	return {};
}

////////////////////////////////////////
auto evaluateClassDefinition(Instance &vm_, rigc::ParserNode const& expr_) -> OptValue
{
	// auto const templateParamList = getTemplateParamList(expr_);
	// std::pair<String, TypeConstraint>, string is a name,
	// TypeConstraint is, for now, a struct with just a name (String)
	// TODO: actually do something with the template parameter list

	auto type = std::make_shared<ClassType>();
	type->parse(expr_);
	vm_.currentScope->addType(type);

	auto prevClass = vm_.currentClass;
	vm_.currentClass = type.get();

	// Evaluate the class body
	auto body = findElem<rigc::ClassCodeBlock>(expr_, false);
	for (auto const& child : body->children)
	{
		vm_.evaluate(*child);
	}

	type->postEvaluate(vm_);

	vm_.currentClass = prevClass;

	return {};
}



////////////////////////////////////////
auto evaluateDataMemberDefinition(Instance &vm_, rigc::ParserNode const& expr_) -> OptValue
{
	auto typeExpr	= findElem<rigc::ExplicitType>(expr_, false);
	auto varName	= findElem<rigc::Name>(expr_, false)->string_view();
	auto valueExpr	= findElem<rigc::InitializerValue>(expr_, false);

	// TODO: this is a quick implementation, it should remain temporary and be remade later
	if(auto const enumType = vm_.currentClass->as<EnumType>()) {
		auto const type = enumType->underlyingType;
		auto member = DataMember{ String(varName), std::move(type) };

		if(valueExpr)
			enumType->add(std::move(member), ExpressionExecutor(vm_, *valueExpr).evaluate());
		else
			enumType->add(std::move(member), std::nullopt);

		return {};
	}

	// bool deduceType = (typeExpr == nullptr);

	// Value value;
	// if (valueExpr) {
	// 	value = vm_.evaluate(*valueExpr).value();
	// }
	// else if (deduceType) {
	// 	throw std::runtime_error(
	// 			fmt::format("Variable {} requires an initializer, because of type deduction using \"{}\"", varName, declType)
	// 		);
	// }

	DeclType type;
	// if (deduceType)
	// 	type = value.type;
	// else
	{
		auto& declType = *findElem<rigc::Type>(*typeExpr, false);

		type = vm_.evaluateType(declType);
		if (!type)
			throw RigCError("Type {} not found", declType.string_view())
							.withHelp("Check the spelling of the type.")
							.withLine(vm_.lastEvaluatedLine);

		// if (!valueExpr)
		// {
		// 	value = vm_.allocateOnStack(type, nullptr);
		// }
		// else if (value.type != type)
		// {
		// 	auto converted = vm_.tryConvert(value, type);
		// 	if (!converted)
		// 		throw std::runtime_error(fmt::format("Cannot convert {} to {}", value.typeName(), type->name()));

		// 	value = converted.value();
		// }
	}


	// value = vm_.cloneValue(value);

	if(auto unionType = vm_.currentClass->as<UnionType>())
		unionType->add( DataMember{ String(varName), std::move(type) }, valueExpr);
	else if(auto classType = vm_.currentClass->as<ClassType>())
		classType->add( DataMember{ String(varName), std::move(type) }, valueExpr);

	return {};
}
}
