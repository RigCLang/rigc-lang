#pragma once

#include RIGCVM_PCH

#include <RigCVM/Value.hpp>
#include <RigCVM/ExtendedVariant.hpp>
#include <RigCVM/Functions.hpp>

namespace rigc::vm
{

struct Instance;
struct ProcessedFunction {
	FunctionCandidates	candidates;
	OptValue			self={};
	StringView	name={};
};

class ExpressionExecutor
{
public:
	using PendingAction		= rigc::ParserNode*;
	using RuntimeValue		= OptValue;

	using ProcessedAction	= ExtendedVariant<RuntimeValue, ProcessedFunction>;
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

	auto evalSingleAction(Action& lhs_) -> ProcessedAction;

	auto evalInfixOperator(StringView op_, Action& lhs_, Action& rhs_) -> ProcessedAction;
	auto evalPrefixOperator(StringView op_, Action& rhs_) -> ProcessedAction;
	auto evalPostfixOperator(rigc::ParserNode const& op_, Action& lhs_) -> ProcessedAction;

	std::vector<Action> actions;
};


}
