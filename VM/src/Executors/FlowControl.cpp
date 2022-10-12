#include RIGCVM_PCH

#include <RigCVM/Executors/All.hpp>
#include <RigCVM/VM.hpp>

namespace rigc::vm
{
////////////////////////////////////////
auto executeReturnStatement(Instance &vm_, rigc::ParserNode const& stmt_) -> OptValue
{
	auto expr = findElem<rigc::Expression>(stmt_);

	OptValue retVal;
	if (expr)
		retVal = vm_.evaluate(*expr);

	vm_.returnTriggered = true;
	return retVal;
}

////////////////////////////////////////
auto evaluateBreakStatement(Instance &vm_, rigc::ParserNode const& stmt_) -> OptValue
{
	auto const breakLevel = findElem<IntegerLiteral>(stmt_, false);

	if(!breakLevel)
		vm_.breakLevel = 1;
	else
		vm_.breakLevel = std::stoi( breakLevel->string()  );

	return {};
}

////////////////////////////////////////
auto evaluateContinueStatement(Instance &vm_, rigc::ParserNode const& stmt_) -> OptValue
{
	vm_.continueTriggered = true;
	return {};
}

////////////////////////////////////////
auto executeIfStatement(Instance &vm_, rigc::ParserNode const& stmt_) -> OptValue
{
	auto& cond = *findElem<rigc::Condition>(stmt_, false);
	auto& expr = *findElem<rigc::Expression>(cond, false);

	auto body = findElem<rigc::CodeBlock>(stmt_, false);
	if (!body)
		body = findElem<rigc::SingleBlockStatement>(stmt_, false);

	auto result = vm_.evaluate(expr);

	if (result.has_value() && result->safeRemoveRef().view<bool>() == true)
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
auto executeWhileStatement(Instance &vm_, rigc::ParserNode const& stmt_) -> OptValue
{
	auto& cond = *findElem<rigc::Condition>(stmt_, false);
	auto& expr = *findElem<rigc::Expression>(cond, false);

	auto body = findElem<rigc::CodeBlock>(stmt_, false);
	if (!body)
		body = findElem<rigc::SingleBlockStatement>(stmt_, false);

	while (true)
	{
		auto scope = StackFramePusher(vm_, *body);

		auto result = vm_.evaluate(expr);

		if (result.has_value() && result->safeRemoveRef().view<bool>())
		{
			auto ret = vm_.evaluate(*body);

			if (vm_.returnTriggered) return ret;
			if(vm_.breakLevel) {
				vm_.breakLevel--;
				break;
			}
			else if(vm_.continueTriggered) {
				vm_.continueTriggered = false;
				continue;
			}
		}
		else
			break;
	}

	return {};
}

////////////////////////////////////////
auto executeForStatement(Instance &vm_, rigc::ParserNode const& stmt_) -> OptValue
{
	auto body = findElem<rigc::CodeBlock>(stmt_, false);
	if (!body)
		body = findElem<rigc::SingleBlockStatement>(stmt_, false);

	auto& variableDef = *findElem<rigc::VariableDefinition>(stmt_, false);
	vm_.evaluate(variableDef);

	auto const& conditionExpr = *findElem<rigc::Expression>(stmt_, false);
	auto const& incrementExpr = *findNthElem<rigc::Expression>(stmt_, 2);

	while (true)
	{
		auto scope = StackFramePusher(vm_, *body);
		auto const conditionResult = vm_.evaluate(conditionExpr);

		if (conditionResult.has_value() && conditionResult->safeRemoveRef().view<bool>())
		{
			auto ret = vm_.evaluate(*body);

			if (vm_.returnTriggered) return ret;
		}
		else
			break;

		vm_.evaluate(incrementExpr);
		if(vm_.breakLevel)
		{
			vm_.breakLevel--;
			break;
		}
		else if(vm_.continueTriggered)
		{
			vm_.continueTriggered = false;
			continue;
		}
	}

	return {};
}
}
