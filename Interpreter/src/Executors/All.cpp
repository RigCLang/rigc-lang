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
	MAKE_EXECUTOR(Expression,			evaluateExpression),
	MAKE_EXECUTOR(FunctionCall,			evaluateFunctionCall),
	MAKE_EXECUTOR(Name,					evaluateName),
	MAKE_EXECUTOR(IntegerLiteral,		evaluateIntegerLiteral),
	// MAKE_EXECUTOR(StringLiteral,		evaluateStringLiteral),
	// MAKE_EXECUTOR(ArrayLiteral,			evaluateArrayLiteral),
	// MAKE_EXECUTOR(ArrayElement,			evaluateArrayElement),
	MAKE_EXECUTOR(VariableDefinition,	evaluateVariableDefinition),
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
				return val;
		}
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
OptValue executeIfStatement(Instance &inst_, rigc::ParserNode const& stmt_)
{
	auto& cond = *findElem<rigc::Condition>(stmt_, false);
	auto& expr = *findElem<rigc::Expression>(cond, false);

	auto body = findElem<rigc::CodeBlock>(stmt_, false);
	if (!body)
		body = findElem<rigc::Statement>(stmt_, false);

	auto result = inst_.evaluate(expr);

	if (result.has_value() && result.value().as<int>(0) == 1)
	{
		inst_.evaluate(*body);
	}
	else
	{
		if (auto elseStmt = findElem<rigc::ElseStatement>(stmt_, false))
		{
			if (auto ifStmt = findElem<rigc::IfStatement>(*elseStmt, false))
				inst_.evaluate(*ifStmt);
			else if (auto body = findElem<rigc::CodeBlock>(*elseStmt, false))
				inst_.evaluate(*body);
			else
				inst_.evaluate(*findElem<rigc::Statement>(*elseStmt, false));
		}
	}

	return {};
}

////////////////////////////////////////
OptValue executeWhileStatement(Instance &inst_, rigc::ParserNode const& stmt_)
{
	auto& cond = *findElem<rigc::Condition>(stmt_, false);
	auto& expr = *findElem<rigc::Expression>(cond, false);

	auto body = findElem<rigc::CodeBlock>(stmt_, false);
	if (!body)
		body = findElem<rigc::Statement>(stmt_, false);

	auto result = inst_.evaluate(expr);
	while (result.has_value() && result.value().as<bool>(false))
	{	
		inst_.evaluate(*body);

		result = inst_.evaluate(expr);
	}

	return {};
}

////////////////////////////////////////
void print(Instance &inst_, rigc::ParserNode const& args)
{
	for (auto const& c : args.children)
	{
		OptValue optVal = inst_.evaluate(*c);
		if (optVal.has_value())
		{
			Value& val = optVal.value();
			DeclType const& type = val.getType();

			if (val.typeName() == "Int32")
				std::cout << val.view<int>();
			else if (val.typeName() == "float")
				std::cout << val.view<float>();
			else if (val.typeName() == "std::string")
				std::cout << val.view<std::string>();
		}
	}
}

////////////////////////////////////////
OptValue evaluateFunctionCall(Instance &inst_, rigc::ParserNode const& stmt_)
{
	// std::cout << "Calling function " << stmt_.string_view() << std::endl;
	
	auto fnName = findElem<rigc::Name>(stmt_, false);

	if (fnName)
	{
		if (fnName->string_view() == "print")
		{
			auto args = findElem<rigc::ListOfFunctionArguments>(stmt_, false);
			print(inst_, *args);
		}
	}

	return {};
}

////////////////////////////////////////
OptValue evaluateExpression(Instance &inst_, rigc::ParserNode const& expr_)
{
	return ExpressionExecutor{inst_, expr_}.evaluate();
}

////////////////////////////////////////
OptValue evaluateName(Instance &inst_, rigc::ParserNode const& expr_)
{
	Ref<Value> ref = inst_.findVariableByName(expr_.string_view());

	if (!ref) {
		throw std::runtime_error("No variable with name \"" + expr_.string() + "\"");
	}

	Value var(*ref);

	// inst_.stack.push( var );
	return var;
}

////////////////////////////////////////
OptValue evaluateVariableDefinition(Instance &inst_, rigc::ParserNode const& expr_)
{
	auto varName	= findElem<rigc::Name>(expr_, false)->string_view();
	auto valueExpr	= findElem<rigc::InitializerValue>(expr_, false);

	Value value;
	if (valueExpr) {
		value = inst_.evaluate(*valueExpr).value();
	}

	inst_.createVariable(varName, value);

	// inst_.stack.push( value );
	return value;
}

////////////////////////////////////////
OptValue evaluateIntegerLiteral(Instance &inst_, rigc::ParserNode const& expr_)
{
	return inst_.allocateOnStack<int>( "Int32", std::stoi(expr_.string()) );
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
// OptValue evaluateStringLiteral(Instance &inst_, rigc::ParserNode const& expr_)
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

// 	// inst_.stack.push( v );
// 	return v;
// }

// ////////////////////////////////////////
// OptValue evaluateArrayLiteral(Instance &inst_, rigc::ParserNode const& expr_)
// {
// 	std::vector<Value> arr;
// 	arr.reserve(expr_.children.size());

// 	for (auto const& c : expr_.children)
// 	{
// 		arr.push_back( inst_.evaluate(*c).value() );
// 	}

// 	Value v( std::move(arr) );

// 	// inst_.stack.push( v );
// 	return v;
// }

// ////////////////////////////////////////
// OptValue evaluateArrayElement(Instance &inst_, rigc::ParserNode const& expr_)
// {
// 	Value v( inst_.evaluate(*expr_.children[0]).value() );

// 	// inst_.stack.push( v );
// 	return v;
// }


}