#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/VM.hpp>
#include <RigCInterpreter/Executors/ExpressionExecutor.hpp>

namespace rigc::vm
{

////////////////////////////////////////
bool isOperator(rigc::ParserNode const& node_)
{
	return (
		node_.is_type<rigc::InfixOperator>()		||
		node_.is_type<rigc::InfixOperatorNoComma>()	||
		node_.is_type<rigc::PrefixOperator>()		||
		node_.is_type<rigc::PostfixOperator>()
	);
}

////////////////////////////////////////
int operatorPriority(rigc::ParserNode const& node_)
{
	if (node_.string_view() == "=")
		return 999;
	return 1;
}

////////////////////////////////////////////
OptValue ExpressionExecutor::evaluate()
{
	if (ctx.children.size() == 1)
		return vm.evaluate(*ctx.children[0]);

	actions.reserve(ctx.children.size());

	for (size_t i = 0; i < ctx.children.size(); ++i)
	{
		actions.push_back( PendingAction{ ctx.children[i].get() } );
	}

	size_t numPending = actions.size();
	while (numPending > 0)
	{
		size_t bestPriority = 999999;
		size_t bestIdx = 0;

		for (size_t i = 0; i < actions.size(); ++i)
		{
			if ( !std::holds_alternative<PendingAction>(actions[i]) )
				continue;
			PendingAction pa = std::get<PendingAction>(actions[i]);

			if (isOperator(*pa))
			{
				int priority = operatorPriority(*pa);
				if (priority < bestPriority)
				{
					bestIdx			= i;
					bestPriority	= priority;
				}
			}
		}
		
		this->evaluateAction(actions[bestIdx], bestIdx);

		numPending = 0;
		for (size_t i = 0; i < actions.size(); ++i)
		{
			if ( std::holds_alternative<PendingAction>(actions[i]) )
				++numPending;
		}
	}

	return actions[0].as<ProcessedAction>();
}

////////////////////////////////////////
void ExpressionExecutor::evaluateAction(Action &action_, size_t actionIndex_)
{
	rigc::ParserNode const& oper = *std::get<PendingAction>(action_);

	if (oper.is_type<rigc::InfixOperator>() || oper.is_type<rigc::InfixOperatorNoComma>())
	{
		if (actionIndex_ == 0 || actionIndex_ == actions.size() - 1)
			throw std::runtime_error("Invalid infix operator position: " + std::to_string(actionIndex_));
		
		action_ = this->evalInfixOperator(
				oper.string_view(),
				actions[actionIndex_ - 1],
				actions[actionIndex_ + 1]
			);
		actions.erase(actions.begin() + actionIndex_ + 1);
		actions.erase(actions.begin() + actionIndex_ - 1);
	}
	else if (oper.is_type<rigc::PrefixOperator>())
	{
		if (actionIndex_ == actions.size() - 1)
			throw std::runtime_error("Invalid prefix operator position: " + std::to_string(actionIndex_));

		action_ = this->evalPrefixOperator(
				oper.string_view(),
				actions[actionIndex_ + 1]
			);

		actions.erase(actions.begin() + actionIndex_ + 1);
	}
	else if (oper.is_type<rigc::PostfixOperator>())
	{
		if (actionIndex_ == 0)
			throw std::runtime_error("Invalid postfix operator position: " + std::to_string(actionIndex_));

		action_ = this->evalPostfixOperator(
				oper.string_view(),
				actions[actionIndex_ - 1]
			);
		
		actions.erase(actions.begin() + actionIndex_ - 1);
	}
}

////////////////////////////////////////
Value ExpressionExecutor::evalSingleAction(Action & lhs_)
{
	if (lhs_.is<PendingAction>())
		return *vm.evaluate(*lhs_.as<PendingAction>());
	
	return lhs_.as<ProcessedAction>();
}

////////////////////////////////////////
Value ExpressionExecutor::evalInfixOperator(std::string_view op_, Action& lhs_, Action& rhs_)
{
	Value lhs	= this->evalSingleAction(lhs_);
	Value rhs	= this->evalSingleAction(rhs_);

	// if (lhs.valueTypeIndex() != rhs.valueTypeIndex())
	// 	throw std::runtime_error("Incompatible operands for \"" + std::string(op_) + "\" operator.");

	FunctionParamTypes types;
	types[0] = lhs.getType();
	types[1] = rhs.getType();

	if (auto overloads = vm.univeralScope().findOperator(op_, Operator::Infix))
	{
		if (auto func = findOverload(*overloads, types, 2))
		{
			Function::Args args;
			args[0] = lhs;
			args[1] = rhs;
			return func->invoke(vm, args, 2).value();
		}
	}

	throw std::runtime_error("Invalid operator \"" + std::string(op_) + "\" for the type (TODO).");
}

////////////////////////////////////////
Value ExpressionExecutor::evalPrefixOperator(std::string_view op_, Action& rhs_)
{
	return {};
}

////////////////////////////////////////
Value ExpressionExecutor::evalPostfixOperator(std::string_view op_, Action& lhs_)
{
	return {};
}


}