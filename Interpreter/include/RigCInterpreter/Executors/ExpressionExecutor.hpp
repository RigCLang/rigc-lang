#pragma once

#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Value.hpp>

namespace rigc::vm
{

struct Instance;

class ExpressionExecutor
{
public:
	using PendingAction		= rigc::ParserNode*;
	using ProcessedAction	= Value;

	using ActionVariant		= std::variant<PendingAction, ProcessedAction>;

	struct Action
		: ActionVariant
	{
		using ActionVariant::ActionVariant;
		
		template <typename T>
		bool is() const {
			return std::holds_alternative<T>(*this);
		}

		template <typename T>
		T& as() {
			return std::get<T>(*this);
		}

		template <typename T>
		T const& as() const {
			return std::get<T>(*this);
		}
	};

	ExpressionExecutor(Instance& vm_, rigc::ParserNode& ctx_)
		: vm(vm_), ctx(ctx_)
	{
	}

	Instance& vm;
	rigc::ParserNode& ctx;

	OptValue evaluate();

private:

	void evaluateAction(Action &action_, size_t actionIndex_);

	Value evalSingleAction(Action& lhs_);

	Value evalInfixOperator(std::string_view op_, Action& lhs_, Action& rhs_);
	Value evalPrefixOperator(std::string_view op_, Action& rhs_);
	Value evalPostfixOperator(std::string_view op_, Action& lhs_);

	std::vector<Action> actions;
};


}