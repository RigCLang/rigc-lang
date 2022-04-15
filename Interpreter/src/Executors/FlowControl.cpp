#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Executors/All.hpp>
#include <RigCInterpreter/Executors/ExpressionExecutor.hpp>
#include <RigCInterpreter/VM.hpp>

#include <RigCInterpreter/TypeSystem/ArrayType.hpp>
#include <RigCInterpreter/TypeSystem/RefType.hpp>
#include <RigCInterpreter/TypeSystem/ClassType.hpp>

namespace rigc::vm
{


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



}
