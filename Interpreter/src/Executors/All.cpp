#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Executors/All.hpp>
#include <RigCInterpreter/Executors/ExpressionExecutor.hpp>

#include <RigCInterpreter/VM.hpp>

namespace rigc::vm
{

#define MAKE_EXECUTOR(ClassName, Executor) { "struct rigc::" #ClassName, Executor }

std::map<ExecutorTrigger, ExecutorFunction*> Executors = {
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
	// MAKE_EXECUTOR(StringLiteral,		evaluateStringLiteral),
	// MAKE_EXECUTOR(ArrayLiteral,			evaluateArrayLiteral),
	// MAKE_EXECUTOR(ArrayElement,			evaluateArrayElement),
	MAKE_EXECUTOR(VariableDefinition,	evaluateVariableDefinition),
	MAKE_EXECUTOR(FunctionDefinition,	evaluateFunctionDefinition),
	MAKE_EXECUTOR(InitializerValue,		evaluateExpression),
	MAKE_EXECUTOR(FunctionArg,			evaluateExpression),
};

#undef MAKE_EXECUTOR

	
////////////////////////////////////////
OptValue executeCodeBlock(Instance &vm_, rigc::ParserNode const& codeBlock_)
{
	vm_.pushScope();

	auto stmts = findElem<rigc::Statements>(codeBlock_);

	if (stmts)
	{
		for (auto const& stmt : stmts->children)
		{	
			OptValue val = vm_.evaluate(*stmt);
			if (vm_.returnTriggered)
			{
				vm_.popScope();
				return val;
			}
		}
	}

	vm_.popScope();

	return {};
}

////////////////////////////////////////
OptValue executeSingleStatement(Instance &vm_, rigc::ParserNode const& stmt_)
{
	vm_.pushScope();

	for (auto const& childStmt : stmt_.children)
	{	
		OptValue val = vm_.evaluate(*childStmt);
		if (vm_.returnTriggered)
			return val;
	}
	vm_.popScope();

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
		vm_.evaluate(*body);
	}
	else
	{
		if (auto elseStmt = findElem<rigc::ElseStatement>(stmt_, false))
		{
			if (auto ifStmt = findElem<rigc::IfStatement>(*elseStmt, false))
				vm_.evaluate(*ifStmt);
			else if (auto body = findElem<rigc::CodeBlock>(*elseStmt, false))
				vm_.evaluate(*body);
			else
				vm_.evaluate(*findElem<rigc::SingleBlockStatement>(*elseStmt, false));
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

	auto result = vm_.evaluate(expr);
	while (result.has_value() && result.value().view<bool>())
	{	
		vm_.evaluate(*body);

		result = vm_.evaluate(expr);
	}

	return {};
}

////////////////////////////////////////
void print(Instance &vm_, rigc::ParserNode const& args)
{
	for (auto const& c : args.children)
	{
		OptValue optVal = vm_.evaluate(*c);
		if (optVal.has_value())
		{
			Value& val = optVal.value();
			DeclType const& type = val.getType();

			if (val.typeName() == "Int32")
				std::cout << val.view<int>();
			if (val.typeName() == "Bool")
				std::cout << (val.view<bool>() ? "true" : "false");
			else if (val.typeName() == "float")
				std::cout << val.view<float>();
			else if (val.typeName() == "std::string")
				std::cout << val.view<std::string>();
		}
	}
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
					evaluatedArgs[i] = vm_.evaluate(*args->children[i]).value();

			}
			return ( (*overloads)[0]->invoke(vm_, evaluatedArgs, numArgs) );
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
	Ref<Value> ref = vm_.findVariableByName(expr_.string_view());

	if (!ref) {
		throw std::runtime_error("No variable with name \"" + expr_.string() + "\"");
	}

	Value var(*ref);

	// vm_.stack.push( var );
	return var;
}

////////////////////////////////////////
OptValue evaluateVariableDefinition(Instance &vm_, rigc::ParserNode const& expr_)
{
	auto varName	= findElem<rigc::Name>(expr_, false)->string_view();
	auto valueExpr	= findElem<rigc::InitializerValue>(expr_, false);

	Value value;
	if (valueExpr) {
		value = vm_.evaluate(*valueExpr).value();
	}

	vm_.createVariable(varName, value);

	// vm_.stack.push( value );
	return value;
}

////////////////////////////////////////
OptValue evaluateFunctionDefinition(Instance &vm_, rigc::ParserNode const& expr_)
{
	auto& scope = vm_.univeralScope();

	auto name = findElem<rigc::Name>(expr_, false)->string_view();

	Function::Params params;
	size_t numParams = 0;

	auto paramList = findElem<rigc::FunctionParams>(expr_, false);
	if (paramList)
	{
		for (auto const& param : paramList->children)
		{
			params[numParams++] = {
				findElem<rigc::Name>(*param)->string_view(),
				DeclType::fromType(*vm_.findType("Int32"))
			};
		}
	}

	scope.registerFunction(vm_, name, Function(Function::RuntimeFn(&expr_), params, numParams));

	return {};
}

////////////////////////////////////////
OptValue evaluateIntegerLiteral(Instance &vm_, rigc::ParserNode const& expr_)
{
	return vm_.allocateOnStack<int>( "Int32", std::stoi(expr_.string()) );
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

// ////////////////////////////////////////
// OptValue evaluateStringLiteral(Instance &vm_, rigc::ParserNode const& expr_)
// {
// 	auto sv = expr_.string_view();

// 	std::string s(sv, 1, sv.length() - 2);
// 	s.reserve(s.size() * 2);
// 	replaceAll(s, "\\n",	"\n");
// 	replaceAll(s, "\\t",	"\t");
// 	replaceAll(s, "\\r",	"\r");
// 	replaceAll(s, "\\a",	"\a");
// 	replaceAll(s, "\\v",	"\v");
// 	replaceAll(s, "\\\\",	"\\");
// 	replaceAll(s, "\\\"",	"\"");

// 	Value v( std::move(s));

// 	// vm_.stack.push( v );
// 	return v;
// }

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