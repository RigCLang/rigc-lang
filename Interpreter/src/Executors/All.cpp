#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Executors/All.hpp>

#include <RigCInterpreter/VM.hpp>

namespace rigc::vm
{

#define MAKE_EXECUTOR(ClassName, Executor) { "struct rigc::" #ClassName, Executor }

std::map<ExecutorTrigger, ExecutorFunction*> Executors = {
	MAKE_EXECUTOR(CodeBlock,			executeCodeBlock),
	MAKE_EXECUTOR(IfStatement,			executeIfStatement),
	MAKE_EXECUTOR(Expression,			evaluateExpression),
	MAKE_EXECUTOR(FunctionCall,			evaluateFunctionCall),
	MAKE_EXECUTOR(Name,					evaluateName),
	MAKE_EXECUTOR(IntegralLiteral,		evaluateIntegralLiteral),
};

#undef MAKE_EXECUTOR

	
////////////////////////////////////////
OptValue executeCodeBlock(Instance &inst_, rigc::ParserNode& codeBlock_)
{
	auto stmts = findElem<rigc::Statements>(codeBlock_);

	if (stmts)
	{
		for (auto const& stmt : stmts->children)
		{
			inst_.evaluate(*stmt);
		}
	}

	return {};
}

////////////////////////////////////////
OptValue executeIfStatement(Instance &inst_, rigc::ParserNode& stmt_)
{
	auto& cond = *findElem<rigc::Condition>(stmt_, false);
	auto& expr = *findElem<rigc::Expression>(cond, false);

	auto result = inst_.evaluate(expr);

	if (result.has_value() && result.value().val == true)
	{
		auto body = findElem<rigc::CodeBlock>(stmt_, false);
		if (!body)
			body = findElem<rigc::Statement>(stmt_, false);
		
		inst_.evaluate(*body);
	}
	else
	{
		if (auto elseStmt = findElem<rigc::ElseStatement>(stmt_, false))
		{
			if (auto ifStmt = findElem<rigc::IfStatement>(*elseStmt, false))
			{
				inst_.evaluate(*ifStmt);
			}
			else if (auto body = findElem<rigc::CodeBlock>(*elseStmt, false))
			{
				inst_.evaluate(*body);
			}
			else
			{
				inst_.evaluate(*findElem<rigc::Statement>(*elseStmt, false));
			}
		}
	}

	return {};
}

////////////////////////////////////////
OptValue evaluateFunctionCall(Instance &inst_, rigc::ParserNode& stmt_)
{
	std::cout << "Calling function " << stmt_.string_view() << std::endl;
	
	return {};
}

////////////////////////////////////////
OptValue evaluateExpression(Instance &inst_, rigc::ParserNode& expr_)
{
	std::cout << "Evaluating expression: " << expr_.string_view() << std::endl;

	auto const& c = expr_.children;
	for (auto const & subExpr : expr_.children) {
		inst_.evaluate(*subExpr);
	}

	Value result = inst_.stack.top();
	inst_.stack.pop();
	return result;
}

////////////////////////////////////////
OptValue evaluateName(Instance &inst_, rigc::ParserNode& expr_)
{
	Value v( 0 );

	inst_.stack.push( v );
	return v;
}

////////////////////////////////////////
OptValue evaluateIntegralLiteral(Instance &inst_, rigc::ParserNode& expr_)
{
	Value v( std::stoi(expr_.string()) );

	inst_.stack.push( v );
	return v;
}


}