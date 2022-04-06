#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/VM.hpp>
#include <RigCInterpreter/Executors/ExpressionExecutor.hpp>

#include <RigCInterpreter/TypeSystem/ClassType.hpp>
#include <RigCInterpreter/TypeSystem/RefType.hpp>

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
	auto op = node_.string_view();
	if (op == ",") return 17;
	if (op == "=" || op == "+=" || op == "-=" || op == "*=" || op == "/=" || op == "%=")
		return 16;
	if (op == "||") return 15;
	if (op == "&&") return 14;
	if (op == "|") return 13;
	if (op == "^") return 12;
	if (op == "&") return 11;
	if (op == "==" || op == "!=") return 10;
	if (op == "<" || op == ">" || op == "<=" || op == ">=") return 9;
	if (op == "<<" || op == ">>") return 8;
	if (op == "+" || op == "-") return 7;
	if (op == "*" || op == "/" || op == "%") return 6;

	if (op == "++" || op == "--") return 2;


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
		// fmt::print("Infix operator: {} resulted in type {}\n", oper.string_view(), action_.as<ProcessedAction>()->type->name());
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
OptValue ExpressionExecutor::evalSingleAction(Action & lhs_)
{
	if (lhs_.is<PendingAction>())
		return *vm.evaluate(*lhs_.as<PendingAction>());

	return lhs_.as<ProcessedAction>();
}

////////////////////////////////////////
OptValue ExpressionExecutor::evalInfixOperator(std::string_view op_, Action& lhs_, Action& rhs_)
{
	Value lhs	= *this->evalSingleAction(lhs_);

	if (op_ == ".")
	{
		if (!lhs.type->is<RefType>())
			lhs = vm.allocateReference(lhs);

		auto rhsExpr = rhs_.as<PendingAction>();

		if (rhsExpr->is_type<rigc::FunctionCall>())
		{
			auto methodName = findElem<rigc::Name>(*rhsExpr);
			FunctionParamTypes paramTypes;
			size_t numParams = 0;
			paramTypes[numParams++] = lhs.type;

			auto args = findElem<rigc::ListOfFunctionArguments>(*rhsExpr, false);

			Function::Args evaluatedArgs;
			evaluatedArgs[0] = lhs;

			for(size_t i = 0; i < args->children.size(); ++i)
			{
				evaluatedArgs[i + 1]	= vm.evaluate(*args->children[i]).value();
				paramTypes[numParams++]	= evaluatedArgs[i + 1].type;
			}

			auto deref = lhs.removeRef();

			auto overloadsIt = deref.type->methods.find(methodName->string());
			if (overloadsIt == deref.type->methods.end())
				throw std::runtime_error(fmt::format("Method {} not found in type {}.", methodName->string_view(), lhs.type->name()));

			auto fn = findOverload(overloadsIt->second, paramTypes, numParams);

			if (!fn) {
				throw std::runtime_error(fmt::format("Not matching method {} to call with params: {}.", methodName->string_view(), lhs.type->name()));
			}

			// fmt::print("Calling method {} on {}\n", methodName->string_view(), lhs_.as<PendingAction>()->string_view());
			return vm.executeFunction(*fn, evaluatedArgs, numParams);
		}
		else if (rhsExpr->is_type<rigc::Name>())
		{
			lhs = lhs.removeRef();

			auto memberName = rhsExpr->string_view();
			auto type = dynamic_cast<ClassType*>(lhs.type.get());
			if (!type)
				throw std::runtime_error("Can't access member of non-class type.");

			auto memberIt = rg::find(type->dataMembers, memberName, &DataMember::name);
			if (memberIt == type->dataMembers.end())
				throw std::runtime_error(fmt::format("Member {} not found in type {}.", memberName, lhs.type->name()));

			return vm.allocateReference(lhs.member(memberIt->offset, memberIt->type));
		}
		else
			throw std::runtime_error("Invalid expression.");

		return vm.allocateOnStack("Int32", 1);

		// auto overloads = lhs.getType().methods;

		// if (auto func = findOverload(*overloads, types, 2))
		// {
		// 	Function::Args args;
		// 	args[0] = lhs;
		// 	args[1] = rhs;
		// 	return func->invoke(vm, args, 2).value();
		// }
	}
	else
	{
		Value rhs	= *this->evalSingleAction(rhs_);

		// if (lhs.valueTypeIndex() != rhs.valueTypeIndex())
		// 	throw std::runtime_error("Incompatible operands for \"" + std::string(op_) + "\" operator.");

		FunctionParamTypes types;

		size_t typeIdx = 0;
		types[typeIdx++] = lhs.getType();
		types[typeIdx++] = rhs.getType();

		if (auto overloads = vm.universalScope().findOperator(op_, Operator::Infix))
		{
			if (auto func = findOverload(*overloads, types, 2))
			{
				Function::Args args;
				args[0] = lhs;
				args[1] = rhs;
				return vm.executeFunction(*func, args, 2).value();
			}
		}

		throw std::runtime_error(
				"No matching operator \"" + std::string(op_) + "\" for argument types: ( " +
				lhs.fullTypeName() + ", " + rhs.fullTypeName() + " )"
			);
	}
}

////////////////////////////////////////
Value ExpressionExecutor::evalPrefixOperator(std::string_view op_, Action& rhs_)
{
	auto rhs = *this->evalSingleAction(rhs_);

	if (op_ == "*")
	{
		return vm.allocateReference(rhs.safeRemoveRef().removePtr());
	}
	else if (op_ == "&")
		return vm.allocatePointer(rhs);
	else
		throw std::runtime_error("Invalid prefix operator: " + std::string(op_));


	return {};
}

////////////////////////////////////////
Value ExpressionExecutor::evalPostfixOperator(std::string_view op_, Action& lhs_)
{
	return {};
}


}
