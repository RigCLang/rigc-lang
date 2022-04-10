#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Executors/All.hpp>
#include <RigCInterpreter/Executors/ExpressionExecutor.hpp>
#include <RigCInterpreter/VM.hpp>

#include <RigCInterpreter/TypeSystem/ArrayType.hpp>
#include <RigCInterpreter/TypeSystem/RefType.hpp>
#include <RigCInterpreter/TypeSystem/ClassType.hpp>

namespace rigc::vm
{

#define MAKE_EXECUTOR(ClassName, Executor) { #ClassName, Executor }

std::map<ExecutorTrigger, ExecutorFunction*, std::less<> > Executors = {
	MAKE_EXECUTOR(ImportStatement,		executeImportStatement),
	MAKE_EXECUTOR(CodeBlock,			executeCodeBlock),
	MAKE_EXECUTOR(IfStatement,			executeIfStatement),
	MAKE_EXECUTOR(WhileStatement,		executeWhileStatement),
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


struct StackFramePusher
{
	StackFramePusher(Instance& vm_, ParserNode const& stmt_)
		: vm(vm_)
	{
		vm.pushStackFrameOf(&stmt_);
	}

	~StackFramePusher()
	{
		if (!vm.returnTriggered)
			vm.popStackFrame();
	}

	Instance& vm;
};

////////////////////////////////////////
OptValue executeImportStatement(Instance &vm_, rigc::ParserNode const& stmt_)
{
	fmt::print("Importing module {}...\n\n", findElem<rigc::PackageImportFullName>(stmt_)->string_view());
	// TODO: import the module.
	return {};
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
			if (stmt->type == "struct rigc::Expression")
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
OptValue executeReturnStatement(Instance &vm_, rigc::ParserNode const& stmt_)
{
	auto expr = findElem<rigc::Expression>(stmt_);

	OptValue retVal;
	if (expr)
		retVal = vm_.evaluate(*expr);

	vm_.returnTriggered = true;
	return retVal;
}

////////////////////////////////////////
OptValue executeIfStatement(Instance &vm_, rigc::ParserNode const& stmt_)
{
	auto& cond = *findElem<rigc::Condition>(stmt_, false);
	auto& expr = *findElem<rigc::Expression>(cond, false);

	auto body = findElem<rigc::CodeBlock>(stmt_, false);
	if (!body)
		body = findElem<rigc::SingleBlockStatement>(stmt_, false);

	auto result = vm_.evaluate(expr);

	if (result.has_value() && result.value().view<bool>() == true)
	{
		auto ret = vm_.evaluate(*body);
		if (vm_.returnTriggered)
			return ret;
	}
	else
	{
		if (auto elseStmt = findElem<rigc::ElseStatement>(stmt_, false))
		{
			if (auto ifStmt = findElem<rigc::IfStatement>(*elseStmt, false))
				return vm_.evaluate(*ifStmt);
			else if (auto body = findElem<rigc::CodeBlock>(*elseStmt, false))
				return vm_.evaluate(*body);
			else
				return vm_.evaluate(*findElem<rigc::SingleBlockStatement>(*elseStmt, false));
		}
	}

	return {};
}

////////////////////////////////////////
OptValue executeWhileStatement(Instance &vm_, rigc::ParserNode const& stmt_)
{
	auto& cond = *findElem<rigc::Condition>(stmt_, false);
	auto& expr = *findElem<rigc::Expression>(cond, false);

	auto body = findElem<rigc::CodeBlock>(stmt_, false);
	if (!body)
		body = findElem<rigc::SingleBlockStatement>(stmt_, false);

	while (true)
	{
		StackFramePusher scope(vm_, *body);

		auto result = vm_.evaluate(expr);

		if (result.has_value() && result.value().view<bool>())
		{
			auto ret = vm_.evaluate(*body);
			if (vm_.returnTriggered)
				return ret;
		}
		else
			break;
	}

	return {};
}

////////////////////////////////////////
OptValue typeOf(Instance &vm_, rigc::ParserNode const& args)
{
	for (auto const& c : args.children)
	{
		OptValue optVal = vm_.evaluate(*c);
		if (optVal.has_value())
		{
			auto name = optVal.value().fullTypeName();
			auto t = wrap<ArrayType>(vm_.universalScope(), vm_.findType("Char")->shared_from_this(), name.size());

			return vm_.allocateOnStack( t, name.data(), name.size() );
		}
	}
	return std::nullopt;
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

	if (auto ref = dynamic_cast<RefType*>(opt->type.get()))
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
		if (auto ref = dynamic_cast<RefType*>(value.type.get()))
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
				throw std::runtime_error(fmt::format("Cannot convert {} to {}", value.typeName(), type->name()));

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
DeclType evaluateType(Instance& vm_, rigc::ParserNode const& typeNode_)
{
	DeclType evaluatedType;
	auto typeName		= findElem<rigc::Name>(typeNode_)->string_view();
	auto templateParams = findElem<rigc::TemplateParams>(typeNode_);

	if (templateParams)
	{
		if (typeName == "Ref")
		{
			auto inner = findElem<rigc::TemplateParam>(*templateParams);
			return wrap<RefType>(
					vm_.universalScope(),
					evaluateType(vm_, *findElem<rigc::Type>(*inner))
				);
		}
		else if (typeName == "Addr")
		{
			auto inner = findElem<rigc::TemplateParam>(*templateParams);
			return wrap<AddrType>(
					vm_.universalScope(),
					evaluateType(vm_, *findElem<rigc::Type>(*inner))
				);
		}
		else if (typeName == "StaticArray")
		{
			// ensure 2 template params
			if (templateParams->children.size() != 2)
				throw std::runtime_error("StaticArray requires 2 template params: StaticArray<T, Int32 N>");

			auto inner	= evaluateType(vm_, *findElem<rigc::Type>(*templateParams->children[0]));
			// TEMP:
			auto size	= std::stoi(templateParams->children[1]->string());

			return wrap<ArrayType>(vm_.universalScope(), inner, size);
		}
	}

	return vm_.findType(typeName)->shared_from_this();
}

////////////////////////////////////////
void evaluateFunctionParams(Instance& vm_, rigc::ParserNode const& paramsNode_, Function::Params& params_, size_t& numParams_)
{
	for (auto const& param : paramsNode_.children)
	{
		auto paramName	= findElem<rigc::Name>(*param)->string_view();
		auto type		= findElem<rigc::Type>(*param);

		params_[numParams_++] = { paramName, evaluateType(vm_, *type) };
	}
}


////////////////////////////////////////
OptValue evaluateFunctionDefinition(Instance &vm_, rigc::ParserNode const& expr_)
{
	auto& scope = *vm_.currentScope;

	auto name = findElem<rigc::Name>(expr_, false)->string_view();

	Function::Params params;
	size_t numParams = 0;

	auto paramList = findElem<rigc::FunctionParams>(expr_, false);
	if (paramList)
		evaluateFunctionParams(vm_, *paramList, params, numParams);

	scope.registerFunction(vm_, name, Function(Function::RuntimeFn(&expr_), params, numParams));

	return {};
}

////////////////////////////////////////
OptValue evaluateIntegerLiteral(Instance &vm_, rigc::ParserNode const& expr_)
{
	return vm_.allocateOnStack<int>( "Int32", std::stoi(expr_.string()) );
}

////////////////////////////////////////
OptValue evaluateFloat32Literal(Instance &vm_, rigc::ParserNode const& expr_)
{
	auto n = expr_.string();
	return vm_.allocateOnStack<float>( "Float32", std::stof( n.substr(0, n.size() - 1) ) );
}

////////////////////////////////////////
OptValue evaluateFloat64Literal(Instance &vm_, rigc::ParserNode const& expr_)
{
	return vm_.allocateOnStack<double>( "Float64", std::stod(expr_.string()) );
}

////////////////////////////////////////
OptValue evaluateBoolLiteral(Instance &vm_, rigc::ParserNode const& expr_)
{
	return vm_.allocateOnStack<bool>( "Bool", expr_.string_view()[0] == 't' ? true : false);
}

////////////////////////////////////////
void replaceAll(std::string& s, std::string_view from, std::string_view to)
{
	size_t startPos = 0;
	while((startPos = s.find(from, startPos)) != std::string::npos) {
		s.replace(startPos, from.length(), to);
		startPos += to.length();
	}
}

////////////////////////////////////////
OptValue evaluateStringLiteral(Instance &vm_, rigc::ParserNode const& expr_)
{
	auto sv = expr_.string_view();

	std::string s(sv, 1, sv.length() - 2);
	s.reserve(s.size() * 2);
	replaceAll(s, "\\n",	"\n");
	replaceAll(s, "\\t",	"\t");
	replaceAll(s, "\\r",	"\r");
	replaceAll(s, "\\a",	"\a");
	replaceAll(s, "\\v",	"\v");
	replaceAll(s, "\\\\",	"\\");
	replaceAll(s, "\\\"",	"\"");

	auto type = wrap<ArrayType>(vm_.universalScope(), vm_.findType("Char")->shared_from_this(), s.size());

	vm_.universalScope().types.add(type);

	return vm_.allocateOnStack( std::move(type), s.data(), s.size() );
}

////////////////////////////////////////
OptValue evaluateCharLiteral(Instance &vm_, rigc::ParserNode const& expr_)
{
	auto sv = expr_.string_view();

	std::string s(sv, 1, sv.length() - 2);
	replaceAll(s, "\\n",	"\n");
	replaceAll(s, "\\t",	"\t");
	replaceAll(s, "\\r",	"\r");
	replaceAll(s, "\\a",	"\a");
	replaceAll(s, "\\v",	"\v");
	replaceAll(s, "\\\\",	"\\");
	replaceAll(s, "\\\"",	"\"");
	char c = s[0];

	return vm_.allocateOnStack( vm_.findType("Char")->shared_from_this(), c );
}

////////////////////////////////////////
OptValue evaluateClassDefinition(Instance &vm_, rigc::ParserNode const& expr_)
{
	auto type = std::make_shared<ClassType>();
	type->parse(expr_);
	vm_.currentScope->types.add(type);

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
OptValue evaluateMethodDefinition(Instance &vm_, rigc::ParserNode const& expr_)
{
	auto& scope = vm_.scopeOf(vm_.currentClass->declaration);

	auto name = findElem<rigc::Name>(expr_, false)->string_view();

	Function::Params params;
	size_t numParams = 0;
	params[numParams++] = {
		"self",
		wrap<RefType>(vm_.universalScope(), vm_.currentClass->shared_from_this())
	};

	auto paramList = findElem<rigc::FunctionParams>(expr_, false);
	if (paramList)
		evaluateFunctionParams(vm_, *paramList, params, numParams);

	auto& method = scope.registerFunction(vm_, name, Function(Function::RuntimeFn(&expr_), params, numParams));
	vm_.currentClass->methods[name].push_back(&method);
	method.outerType = vm_.currentClass;

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

// ////////////////////////////////////////
// OptValue evaluateArrayLiteral(Instance &vm_, rigc::ParserNode const& expr_)
// {
// 	std::vector<Value> arr;
// 	arr.reserve(expr_.children.size());

// 	for (auto const& c : expr_.children)
// 	{
// 		arr.push_back( vm_.evaluate(*c).value() );
// 	}

// 	Value v( std::move(arr) );

// 	// vm_.stack.push( v );
// 	return v;
// }

// ////////////////////////////////////////
// OptValue evaluateArrayElement(Instance &vm_, rigc::ParserNode const& expr_)
// {
// 	Value v( vm_.evaluate(*expr_.children[0]).value() );

// 	// vm_.stack.push( v );
// 	return v;
// }


}
