#include RIGCVM_PCH

#include <RigCVM/Executors/All.hpp>
#include <RigCVM/Executors/ExpressionExecutor.hpp>
#include <RigCVM/VM.hpp>

#include <RigCVM/TypeSystem/ArrayType.hpp>
#include <RigCVM/TypeSystem/RefType.hpp>
#include <RigCVM/TypeSystem/ClassType.hpp>

namespace rigc::vm
{

#define MAKE_EXECUTOR(ClassName, Executor) { #ClassName, Executor }

std::map<ExecutorTrigger, ExecutorFunction*, std::less<> > Executors = {
	MAKE_EXECUTOR(ImportStatement,		executeImportStatement),
	MAKE_EXECUTOR(CodeBlock,			executeCodeBlock),
	MAKE_EXECUTOR(IfStatement,			executeIfStatement),
	MAKE_EXECUTOR(WhileStatement,		executeWhileStatement),
	MAKE_EXECUTOR(ForStatement,		executeForStatement),
	MAKE_EXECUTOR(ReturnStatement,		executeReturnStatement),
	MAKE_EXECUTOR(SingleBlockStatement,	executeSingleStatement),
	MAKE_EXECUTOR(Expression,			evaluateExpression),
	MAKE_EXECUTOR(Name,					evaluateName),
	MAKE_EXECUTOR(IntegerLiteral,		evaluateIntegerLiteral),
	MAKE_EXECUTOR(BoolLiteral,			evaluateBoolLiteral),
	MAKE_EXECUTOR(StringLiteral,		evaluateStringLiteral),
	MAKE_EXECUTOR(CharLiteral,			evaluateCharLiteral),
	MAKE_EXECUTOR(Float32Literal,		evaluateFloat32Literal),
	MAKE_EXECUTOR(Float64Literal,		evaluateFloat64Literal),
	// MAKE_EXECUTOR(ArrayLiteral,			evaluateArrayLiteral),
	// MAKE_EXECUTOR(ArrayElement,			evaluateArrayElement),
	MAKE_EXECUTOR(VariableDefinition,	evaluateVariableDefinition),
	MAKE_EXECUTOR(FunctionDefinition,	evaluateFunctionDefinition),
	MAKE_EXECUTOR(InitializerValue,		evaluateExpression),
	MAKE_EXECUTOR(FunctionArg,			evaluateExpression),
	MAKE_EXECUTOR(ClassDefinition,		evaluateClassDefinition),
	MAKE_EXECUTOR(MethodDef,			evaluateMethodDefinition),
	MAKE_EXECUTOR(DataMemberDef,		evaluateDataMemberDefinition),
};

#undef MAKE_EXECUTOR

////////////////////////////////////////
StackFramePusher::StackFramePusher(Instance& vm_, ParserNode const& stmt_)
	: vm(vm_)
{
	vm.pushStackFrameOf(&stmt_);
}

////////////////////////////////////////
StackFramePusher::~StackFramePusher()
{
	if (!vm.returnTriggered)
		vm.popStackFrame();
}


////////////////////////////////////////
OptValue executeCodeBlock(Instance &vm_, rigc::ParserNode const& codeBlock_)
{
	StackFramePusher scope(vm_, codeBlock_);

	auto stmts = findElem<rigc::Statements>(codeBlock_);

	if (stmts)
	{
		for (auto const& stmt : stmts->children)
		{
			auto stackFramePos = vm_.stack.size;
			OptValue val = vm_.evaluate(*stmt);
			if (stmt->is_type<rigc::Expression>())
				vm_.stack.size = stackFramePos;

			if (vm_.returnTriggered)
				return val;
		}
	}

	return {};
}

////////////////////////////////////////
OptValue executeSingleStatement(Instance &vm_, rigc::ParserNode const& stmt_)
{
	StackFramePusher scope(vm_, stmt_);

	for (auto const& childStmt : stmt_.children)
	{
		auto ret = vm_.evaluate(*childStmt);
		if (vm_.returnTriggered)
			return ret;
	}

	return {};
}


////////////////////////////////////////
OptValue evaluateExpression(Instance &vm_, rigc::ParserNode const& expr_)
{
	return ExpressionExecutor{vm_, expr_}.evaluate();
}

////////////////////////////////////////
OptValue evaluateName(Instance &vm_, rigc::ParserNode const& expr_)
{
	auto opt = vm_.findVariableByName(expr_.string_view());

	if (!opt) {
		opt = vm_.findFunctionExpr(expr_.string_view());
	}

	if (!opt) {
		throw std::runtime_error("Unrecognized identifier with name \"" + expr_.string() + "\"");
	}

	if (auto ref = opt->type->as<RefType>())
		return opt;

	return vm_.allocateReference(opt.value());
}

////////////////////////////////////////
OptValue evaluateVariableDefinition(Instance &vm_, rigc::ParserNode const& expr_)
{
	auto declType	= findElem<rigc::DeclType>(expr_, false)->string_view();
	auto varName	= findElem<rigc::Name>(expr_, false)->string_view();
	auto valueExpr	= findElem<rigc::InitializerValue>(expr_, false);

	bool deduceType = (declType == "var" || declType == "const");

	Value value;
	if (valueExpr) {
		value = vm_.evaluate(*valueExpr).value();
		if (auto ref = value.type->as<RefType>())
		{
			value = value.removeRef();
		}
	}
	else if (deduceType) {
		throw std::runtime_error(
				fmt::format("Variable {} requires an initializer, because of type deduction using \"{}\"", varName, declType)
			);
	}

	DeclType type;
	if (deduceType)
		type = value.type;
	else
	{
		if (auto t = vm_.findType(declType))
			type = t->shared_from_this();
		else
			throw std::runtime_error(fmt::format("Type {} not found", declType));

		if (!valueExpr)
		{
			value = vm_.allocateOnStack(type, nullptr);

			if (auto c = type->as<ClassType>())
			{
				if (auto ctor = c->defaultConstructor())
				{
					Function::Args args;
					args[0] = vm_.allocateReference(value);
					ctor->invoke(vm_, args, 1);
				}
			}
		}
		else if (value.type != type)
		{
			auto converted = vm_.tryConvert(value, type);
			if (!converted)
				throw std::runtime_error(fmt::format("Cannot convert {} to {}", value.type->name(), type->name()));

			value = converted.value();
		}
	}


	value = vm_.cloneValue(value);

	if (!vm_.currentScope->variables.contains(varName))
	{
		auto varNameStr = std::string(varName);
		auto& var = vm_.currentScope->variables[varNameStr];
		var = vm_.reserveOnStack(type, true);
	}

	return value;
}

////////////////////////////////////////
OptValue evaluateClassDefinition(Instance &vm_, rigc::ParserNode const& expr_)
{
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

	vm_.currentClass = prevClass;

	return {};
}



////////////////////////////////////////
OptValue evaluateDataMemberDefinition(Instance &vm_, rigc::ParserNode const& expr_)
{
	auto typeExpr	= findElem<rigc::ExplicitType>(expr_, false);
	auto varName	= findElem<rigc::Name>(expr_, false)->string_view();
	auto valueExpr	= findElem<rigc::InitializerValue>(expr_, false);

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
		auto declType = findElem<rigc::Type>(*typeExpr, false)->string_view();
		if (auto t = vm_.findType(declType))
			type = t->shared_from_this();
		else
			throw std::runtime_error(fmt::format("Type {} not found", declType));

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

	vm_.currentClass->add({ std::string(varName), std::move(type) });

	return {};
}

}
