#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Executors/All.hpp>
#include <RigCInterpreter/Executors/ExpressionExecutor.hpp>
#include <RigCInterpreter/VM.hpp>

#include <RigCInterpreter/TypeSystem/ArrayType.hpp>
#include <RigCInterpreter/TypeSystem/ClassType.hpp>

namespace rigc::vm
{

#define MAKE_EXECUTOR(ClassName, Executor) { #ClassName, Executor }

std::map<ExecutorTrigger, ExecutorFunction*, std::less<> > Executors = {
	MAKE_EXECUTOR(CodeBlock,			executeCodeBlock),
	MAKE_EXECUTOR(IfStatement,			executeIfStatement),
	MAKE_EXECUTOR(WhileStatement,		executeWhileStatement),
	MAKE_EXECUTOR(ReturnStatement,		executeReturnStatement),
	MAKE_EXECUTOR(SingleBlockStatement,	executeSingleStatement),
	MAKE_EXECUTOR(Expression,			evaluateExpression),
	MAKE_EXECUTOR(FunctionCall,			evaluateFunctionCall),
	MAKE_EXECUTOR(Name,					evaluateName),
	MAKE_EXECUTOR(IntegerLiteral,		evaluateIntegerLiteral),
	MAKE_EXECUTOR(BoolLiteral,			evaluateBoolLiteral),
	MAKE_EXECUTOR(StringLiteral,		evaluateStringLiteral),
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


struct ScopePusher
{
	ScopePusher(Instance& vm_, ParserNode const& stmt_)
		: vm(vm_)
	{
		vm.pushScope(&stmt_);
	}

	~ScopePusher() { vm.popScope(); }

	Instance& vm;
};

////////////////////////////////////////
OptValue executeCodeBlock(Instance &vm_, rigc::ParserNode const& codeBlock_)
{
	ScopePusher scope(vm_, codeBlock_);

	auto stmts = findElem<rigc::Statements>(codeBlock_);

	if (stmts)
	{
		for (auto const& stmt : stmts->children)
		{
			OptValue val = vm_.evaluate(*stmt);
			if (vm_.returnTriggered)
			{
				return val;
			}
		}
	}

	return {};
}

////////////////////////////////////////
OptValue executeSingleStatement(Instance &vm_, rigc::ParserNode const& stmt_)
{
	ScopePusher scope(vm_, stmt_);

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
		ScopePusher scope(vm_, *body);

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
void print(Instance &vm_, rigc::ParserNode const& args)
{
	size_t numArgs = args.children.size();
	if (numArgs == 0)
		return;

	OptValue format = vm_.evaluate(*args.children[0]);
	if (!format.has_value() || !format->getType()->isArray() && format->typeName() != "Char")
		return;

	auto chars = &format->view<char const>();
	auto fmtView = std::string_view(chars, format->getType()->size());

	auto store = fmt::dynamic_format_arg_store<fmt::format_context>();

	for (size_t c = 1; c < numArgs; ++c)
	{
		OptValue optVal = vm_.evaluate(*args.children[c]);
		if (optVal.has_value())
		{
			Value& val = optVal.value();
			if (auto ref = dynamic_cast<RefType*>(val.type.get()))
				val = val.deref();
			DeclType const& type = val.getType();

			auto typeName = val.typeName();
			if (typeName == "Int32")
				store.push_back(val.view<int>());
			if (typeName == "Float32")
				store.push_back(val.view<float>());
			if (typeName == "Float64")
				store.push_back(val.view<double>());
			if (typeName == "Bool")
				store.push_back((val.view<bool>() ? "true" : "false"));
			else if (val.getType()->isArray() && typeName == "Char")
			{
				auto chars = &val.view<char const>();

				store.push_back(std::string(chars, type->size()));
			}
		}
	}

	fmt::vprint(fmtView, store);
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
			auto t = wrap<ArrayType>(vm_.univeralScope().types, vm_.findType("Char")->shared_from_this(), name.size());

			return vm_.allocateOnStack( t, name.data(), name.size() );
		}
	}
	return std::nullopt;
}

////////////////////////////////////////
OptValue evaluateFunctionCall(Instance &vm_, rigc::ParserNode const& stmt_)
{
	// std::cout << "Calling function " << stmt_.string_view() << std::endl;

	auto fnName = findElem<rigc::Name>(stmt_, false);

	if (fnName)
	{
		auto args = findElem<rigc::ListOfFunctionArguments>(stmt_, false);
		if (fnName->string_view() == "print")
		{
			print(vm_, *args);
		}
		else if (fnName->string_view() == "typeOf")
		{
			return typeOf(vm_, *args);
		}
		else
		{
			auto overloads = vm_.findFunction(fnName->string_view());

			if (!overloads)
				throw std::runtime_error("Function " + fnName->string() + " not found");

			Function::Args evaluatedArgs;
			size_t numArgs = 0;
			if (args)
			{
				numArgs = args->children.size();
				for(size_t i = 0; i < numArgs; ++i)
				{
					// fmt::print("Evaluating: {}\n", args->children[i]->string_view());
					evaluatedArgs[i] = vm_.evaluate(*args->children[i]).value();
				}
			}

			FunctionParamTypes types;
			for (size_t i = 0; i < numArgs; ++i)
				types[i] = evaluatedArgs[i].getType();

			auto func = findOverload(*overloads, types, numArgs);
			if (func)
				return func->invoke(vm_, evaluatedArgs, numArgs);
			else
				throw std::runtime_error("Function " + fnName->string() + " not found");
		}
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
	auto optValue = vm_.findVariableByName(expr_.string_view());

	if (!optValue) {
		throw std::runtime_error("No variable with name \"" + expr_.string() + "\"");
	}

	if (auto ref = dynamic_cast<RefType*>(optValue->type.get()))
		return optValue;

	return vm_.allocateReference(optValue.value());
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
			value = value.deref();
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
		vm_.currentScope->variables[std::string(varName)] = vm_.reserveOnStack(type, true);
	}

	return value;
}

////////////////////////////////////////
void evaluateFunctionParams(Instance& vm_, rigc::ParserNode const& paramsNode_, Function::Params& params_, size_t& numParams_)
{
	for (auto const& param : paramsNode_.children)
	{
		auto paramName	= findElem<rigc::Name>(*param)->string_view();
		auto declType	= findElem<rigc::Type>(*param);
		auto typeName	= declType ? declType->string_view() : "Int32";

		params_[numParams_++] = {
			paramName,
			vm_.findType(typeName)->shared_from_this()
		};
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

	auto type = wrap<ArrayType>(vm_.univeralScope().types, vm_.findType("Char")->shared_from_this(), s.size());

	vm_.univeralScope().types.add(type);

	return vm_.allocateOnStack( std::move(type), s.data(), s.size() );
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
	auto& scope = *vm_.currentScope;

	auto name = findElem<rigc::Name>(expr_, false)->string_view();

	Function::Params params;
	size_t numParams = 0;
	params[numParams++] = {
		"self",
		wrap<RefType>(vm_.univeralScope().types, vm_.currentClass->shared_from_this())
	};

	auto paramList = findElem<rigc::FunctionParams>(expr_, false);
	if (paramList)
		evaluateFunctionParams(vm_, *paramList, params, numParams);

	auto& method = scope.registerFunction(vm_, name, Function(Function::RuntimeFn(&expr_), params, numParams));
	vm_.currentClass->methods[name].push_back(&method);

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
