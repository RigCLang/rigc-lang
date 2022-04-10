#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/VM.hpp>
#include <RigCInterpreter/Executors/ExpressionExecutor.hpp>

#include <RigCInterpreter/TypeSystem/ClassType.hpp>
#include <RigCInterpreter/TypeSystem/RefType.hpp>
#include <RigCInterpreter/TypeSystem/FuncType.hpp>

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

	if (op == "++" || op == "--" || op[0] == '(' || op[0] == '[' || op == ".") return 2;


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
				oper,
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

		if (rhsExpr->is_type<rigc::Name>())
		{
			lhs = lhs.removeRef();

			auto memberName = rhsExpr->string_view();

			auto classType = dynamic_cast<ClassType*>(lhs.type.get());

			if (classType)
			{
				auto dmIt = rg::find(classType->dataMembers, memberName, &DataMember::name);
				if (dmIt != classType->dataMembers.end())
				{
					return vm.allocateReference(lhs.member(dmIt->offset, dmIt->type));
				}
			}

			FunctionOverloads const* overloads;

			auto methodsIt = lhs.type->methods.find(memberName);
			if (methodsIt != lhs.type->methods.end())
				overloads = &methodsIt->second;
			else
			{
				if (!(overloads = vm.findFunction(memberName)))
				{
					throw std::runtime_error(fmt::format("Member {} not found in type {}.", memberName, lhs.type->name()));
				}
			}


			return allocateMethodOverloads(vm, lhs, overloads);
		}
		else
			throw std::runtime_error("Invalid expression.");
	}
	else
	{
		Value rhs	= *this->evalSingleAction(rhs_);

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
		return vm.allocateReference(rhs.safeRemoveRef().removePtr());
	else if (op_ == "&")
		return vm.allocatePointer(rhs);
	else
		throw std::runtime_error("Invalid prefix operator: " + std::string(op_));


	return {};
}

////////////////////////////////////////
OptValue ExpressionExecutor::evalPostfixOperator(rigc::ParserNode const& op_, Action& lhs_)
{
	auto lhs = *this->evalSingleAction(lhs_);

	auto op = op_.string_view();
	if (op[0] == '[') // array access
	{
		auto& expr = *findElem<rigc::Expression>(op_, false);
		auto data = lhs.safeRemoveRef();

		auto index		= vm.evaluate(expr)->safeRemoveRef();
		auto elemType	= data.type->decay();

		Value elem;
		elem.data = (char*)data.data + index.view<int>() * elemType->size();
		elem.type = elemType;

		return vm.allocateReference(elem);
	}
	else if (op[0] == '(') // function call
	{
		auto funcVal = lhs.safeRemoveRef();

		FunctionParamTypes paramTypes;
		size_t numParams = 0;

		Function::Args evaluatedArgs;

		FunctionOverloads const* overloads;

		Value self;
		bool constructor	= false;
		bool method			= false;
		if (funcVal.type->is<FuncType>())
		{
			overloads = funcVal.view<FunctionOverloads const*>();
			if (!overloads)
				throw std::runtime_error("Can't call non-function type.");
		}
		else // Method type
		{
			method = true;

			overloads = funcVal.view<FunctionOverloads const*>();
			if (!overloads)
				throw std::runtime_error("Can't call non-function type.");

			self.data = *((void**)funcVal.data + 1);
			self.type = (*overloads)[0]->params[0].type->as<RefType>()->inner;
			if (!self.data) // this is a constructor
			{
				constructor = true;
				self = vm.allocateOnStack(self.type, nullptr);
			}

			evaluatedArgs[numParams]	= vm.allocateReference(self);
			paramTypes[numParams]		= evaluatedArgs[numParams].type;
			numParams++;
		}


		auto args = findElem<rigc::ListOfFunctionArguments>(op_, false);

		for(size_t i = 0; i < args->children.size(); ++i)
		{
			evaluatedArgs[numParams]	= vm.evaluate(*args->children[i]).value();
			paramTypes[numParams]		= evaluatedArgs[numParams].type;
			numParams++;
		}


		auto fn = findOverload(*overloads, paramTypes, numParams, method);

		if (!fn) {
			throw std::runtime_error(fmt::format("Not matching function to call with params: {}.", lhs.type->name()));
		}

		auto result = vm.executeFunction(*fn, evaluatedArgs, numParams);
		if (constructor)
			return evaluatedArgs[0];
		return result;
	}

	return {};
}


}
