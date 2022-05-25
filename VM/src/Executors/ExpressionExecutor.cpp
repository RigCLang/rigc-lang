#include RIGCVM_PCH

#include <RigCVM/VM.hpp>
#include <RigCVM/Executors/ExpressionExecutor.hpp>

#include <RigCVM/TypeSystem/ClassType.hpp>
#include <RigCVM/TypeSystem/EnumType.hpp>
#include <RigCVM/TypeSystem/RefType.hpp>
#include <RigCVM/TypeSystem/FuncType.hpp>

namespace rigc::vm
{

////////////////////////////////////////
auto isOperator(rigc::ParserNode const& node_) -> bool
{
	return (
		node_.is_type<rigc::InfixOperator>()		||
		node_.is_type<rigc::InfixOperatorNoComma>()	||
		node_.is_type<rigc::PrefixOperator>()		||
		node_.is_type<rigc::PostfixOperator>()
	);
}

////////////////////////////////////////
auto operatorPriority(rigc::ParserNode const& node_) -> int
{
	auto op = node_.string_view();
	if (op == ",") return 18;
	if (op == "as") return 17;
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

	if (op == "++" || op == "--")
	{
		if(node_.is_type<PrefixOperator>()) return 3;
		else return 2;
	}
	if (op[0] == '(' || op[0] == '[' || op == ".") return 2;


	return 1;
}

////////////////////////////////////////////
auto ExpressionExecutor::evaluate() -> OptValue
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
auto ExpressionExecutor::evaluateAction(Action &action_, size_t actionIndex_) -> void
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
auto ExpressionExecutor::evalSingleAction(Action & lhs_) -> OptValue
{
	if (lhs_.is<PendingAction>())
		return *vm.evaluate(*lhs_.as<PendingAction>());

	return lhs_.as<ProcessedAction>();
}

////////////////////////////////////////
auto ExpressionExecutor::evalInfixOperator(std::string_view op_, Action& lhs_, Action& rhs_) -> OptValue
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

			if (auto classType = lhs.type->as<ClassType>())
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
	else if(op_ == "::")
	{
		auto const rhs = rhs_.as<PendingAction>();
		if(!rhs->is_type<rigc::Name>())
			throw std::runtime_error("Rhs of the :: operator should be a valid identifier.");

		auto const source = lhs_.as<PendingAction>();
		if(!source->is_type<rigc::Name>())
			throw std::runtime_error("Lhs of the :: operator should be a valid identifier.");

		auto const memberName = rhs->string();
		auto const sourceName = source->string_view();

		auto const sourceType = vm.findType(sourceName);

		if(!sourceType)
			throw std::runtime_error(fmt::format("No type named \"{}\" found.", sourceName));

		if (auto const enumType = sourceType->as<EnumType>())
		{
			if(enumType->fields.contains(memberName))
				return vm.allocateReference(enumType->fields[memberName]);
			else
			 return {};
		}
		else
		{
			throw std::runtime_error("Infix :: not implemented for anything else than enums for now.");
		}

	}
	else if(op_ == "as")
	{
		auto const rhs = rhs_.as<PendingAction>();
		if(!rhs->is_type<rigc::Name>())
			throw std::runtime_error("Rhs of the conversion operator should be a valid identifier.");

		auto const rhsType = vm.findType(rhs->string_view());
		if(!rhsType)
			throw std::runtime_error("Rhs of the conversion identifier should be a type.");

		return vm.tryConvert(lhs, rhsType->shared_from_this());
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
				lhs.type->name() + ", " + rhs.type->name() + " )"
			);
	}
}

auto executeIncrementDecrement(Instance& vm, std::string_view op, Value& operand, Operator::Type operatorType) {
	FunctionParamTypes types;
	size_t typeIdx = 0;
	types[typeIdx++] = operand.getType();

	if (auto overloads = vm.universalScope().findOperator(op, operatorType))
	{
		if (auto func = findOverload(*overloads, types, 1))
		{
			Function::Args args;
			args[0] = operand;
			return vm.executeFunction(*func, args, 1).value();
		}
	}

	throw std::runtime_error(
			fmt::format(
				"No matching {} operator \"{}\" for argument type: {}.",
				operatorType == Operator::Type::Postfix ? "postfix" : "prefix",
				op,
				operand.type->name()
			)
		);
}


////////////////////////////////////////
auto ExpressionExecutor::evalPrefixOperator(std::string_view op_, Action& rhs_) -> Value
{
	auto rhs = *this->evalSingleAction(rhs_);

	if (op_ == "*")
	{
		auto noRef = rhs.safeRemoveRef();
		auto noPtr = noRef.removePtr();
		auto ref = vm.allocateReference(noPtr);
		return ref;
		// return vm.allocateReference(rhs.safeRemoveRef().removePtr());
	}
	else if (op_ == "&")
	{
		return vm.allocatePointer(rhs);
	}
	else if (op_ == "--" || op_ == "++")
	{
		return executeIncrementDecrement(vm, op_, rhs, Operator::Prefix);
	}
	else
		throw std::runtime_error("Invalid prefix operator: " + std::string(op_));


	return {};
}

////////////////////////////////////////
auto ExpressionExecutor::evalPostfixOperator(rigc::ParserNode const& op_, Action& lhs_) -> OptValue
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
	else if (op == "--" || op == "++")
	{
		return executeIncrementDecrement(vm, op, lhs, Operator::Postfix);
	}

	return {};
}


}
