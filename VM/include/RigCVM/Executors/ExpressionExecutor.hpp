#pragma once

#include RIGCVM_PCH

#include <RigCVM/Value.hpp>
#include <RigCVM/ExtendedVariant.hpp>
#include <RigCVM/Scope.hpp>

namespace rigc::vm
{

struct Instance;

class ExpressionExecutor
{
public:
	using PendingAction		= rigc::ParserNode*;
	using ProcessedAction	= OptValue;
	using Action			= ExtendedVariant<PendingAction, ProcessedAction>;

	ExpressionExecutor(Instance& vm_, rigc::ParserNode const& ctx_)
		: vm(vm_), ctx(ctx_)
	{
	}

	Instance& vm;
	rigc::ParserNode const& ctx;

	OptValue evaluate();

private:

	auto evaluateAction(Action &action_, size_t actionIndex_) -> void;

	auto evalSingleAction(Action& lhs_) -> OptValue;

	auto evalInfixOperator(std::string_view op_, Action& lhs_, Action& rhs_) -> OptValue;
	auto evalPrefixOperator(std::string_view op_, Action& rhs_) -> OptValue;
	auto evalPostfixOperator(rigc::ParserNode const& op_, Action& lhs_) -> OptValue;

	auto evalInfixOperatorNonOverloadable(std::string_view op_, Action& lhs_, Action& rhs_) -> OptValue;
	auto evalFunctionCallOperator(rigc::ParserNode const& op_, Action& lhs_) -> OptValue;

	std::vector<Action> actions;
};


}
