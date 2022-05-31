#include RIGCVM_PCH

#include <RigCVM/VM.hpp>
#include <RigCVM/Executors/ExpressionExecutor.hpp>

#include <RigCVM/TypeSystem/ClassType.hpp>
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

	auto& result = actions[0].as<ProcessedAction>();

	if (result.is<OptValue>())
		return result.as<OptValue>();

	return std::nullopt;
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
auto ExpressionExecutor::evalSingleAction(Action & lhs_) -> ProcessedAction
{
	if (lhs_.is<PendingAction>())
		return *vm.evaluate(*lhs_.as<PendingAction>());

	return lhs_.as<ProcessedAction>();
}

////////////////////////////////////////
auto ExpressionExecutor::evalInfixOperator(std::string_view op_, Action& lhs_, Action& rhs_) -> ProcessedAction
{
	Value lhs = *this->evalSingleAction(lhs_).as<OptValue>();

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

			FunctionCandidates candidates;


			auto methodsIt = lhs.type->methods.find(memberName);
			if (methodsIt != lhs.type->methods.end())
			{
				// TODO: check the scope of the method
				candidates.push_back( { &vm.scopeOf(nullptr), &methodsIt->second } );
			}
			else
			{
				candidates = vm.findFunction(memberName);
			}

			return ProcessedFunction{ candidates, lhs, memberName };
		}
		else
			throw std::runtime_error("Invalid expression.");
	}
	else
	{
		Value rhs = *this->evalSingleAction(rhs_).as<OptValue>();

		FunctionParamTypes types;

		size_t typeIdx = 0;
		types[typeIdx++] = lhs.getType();
		types[typeIdx++] = rhs.getType();

		if (auto overloads = vm.universalScope().findOperator(op_, Operator::Infix))
		{
			if (auto func = findOverload(*overloads, { types.data(), 2 }))
			{
				Function::Args args;
				args[0] = lhs;
				args[1] = rhs;
				return vm.executeFunction(*func, Function::ArgSpan{ args.data(), 2 }).value();
			}
		}

		auto msg = fmt::format("No matching operator \"{}\" found for argument types: ( {}, {} )",
				op_,
				lhs.type->name(),
				rhs.type->name()
			);
		throw std::runtime_error(msg);
	}
}

auto executeIncrementDecrement(Instance& vm, std::string_view op, Value& operand, Operator::Type operatorType) {
	FunctionParamTypes types;
	size_t typeIdx = 0;
	types[typeIdx++] = operand.getType();

	if (auto overloads = vm.universalScope().findOperator(op, operatorType))
	{
		if (auto func = findOverload(*overloads, { types.data(), 1 }))
		{
			Function::Args args;
			args[0] = operand;
			return vm.executeFunction(*func, Function::ArgSpan{ args.data(), 1 }).value();
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
auto ExpressionExecutor::evalPrefixOperator(std::string_view op_, Action& rhs_) -> ProcessedAction
{
	auto rhs = *this->evalSingleAction(rhs_).as<RuntimeValue>();

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
auto ExpressionExecutor::evalPostfixOperator(rigc::ParserNode const& op_, Action& lhs_) -> ProcessedAction
{

	auto op = op_.string_view();
	if (op[0] == '[') // array access
	{
		auto lhs = *this->evalSingleAction(lhs_).as<RuntimeValue>();

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
		auto autoOverloadResolution = false;
		auto candidates	= FunctionCandidates();
		auto self		= OptValue();
		auto fnName		= std::string_view();

		if (lhs_.is<PendingAction>()) // lhs is yet to be processed
		{
			// Supports automatic function overload resultion
			// because name is provided directly before `()` operator.
			// Valid code:
			//
			//    funcWithOverloads(param1, param2);
			//
			// Not matching example:
			//
			//    var func: Ref = funcWithOverloads; // 🔴 Error
			//    func(param1, param2);
			//
			if (auto act = lhs_.as<PendingAction>(); act->is_type<rigc::Name>())
			{
				autoOverloadResolution = true;
				candidates = vm.findFunction(act->string_view());
			}
		}
		else if (auto act = lhs_.as<ProcessedAction>(); act.is<ProcessedFunction>())
		{
			autoOverloadResolution = true;
			auto& processedFunc = act.as<ProcessedFunction>();

			candidates	= processedFunc.candidates;
			self		= processedFunc.self;
			fnName		= processedFunc.name;
		}

		FunctionParamTypes paramTypes;
		size_t numParams = 0;
		size_t paramIdx = 1; // first one is reserved to `self` (ignored if self is not provided)

		Function::Args evaluatedArgs;
		bool method			= self.has_value();
		bool constructor	= false;

		if (!autoOverloadResolution)
		{
			auto lhs = *this->evalSingleAction(lhs_).as<RuntimeValue>();
			auto funcVal = lhs.safeRemoveRef();

			if (funcVal.type->is<FuncType>())
				candidates.push_back( { &vm.scopeOf(nullptr), funcVal.view<FunctionOverloads const*>() } );
		}

		if (self) {
			evaluatedArgs[0]	= vm.allocateReference(*self);
			paramTypes[0]		= evaluatedArgs[0].type;
			++numParams;
		}

		auto args = findElem<rigc::ListOfFunctionArguments>(op_, false);

		for(size_t i = 0; i < args->children.size(); ++i)
		{
			evaluatedArgs[paramIdx]	= vm.evaluate(*args->children[i]).value();
			paramTypes[paramIdx]	= evaluatedArgs[paramIdx].type;
			++numParams;
			++paramIdx;
		}

		auto fn = findOverload(candidates, { paramTypes.data() + (self ? 0 : 1), numParams }, method);

		if (!fn)
		{
			if (fnName.empty() && lhs_.is<PendingAction>())
			{
				auto& lhsExpr = *lhs_.as<PendingAction>();
				if (lhsExpr.is_type<rigc::Name>())
					fnName = lhsExpr.string_view();

			}
			if (!fnName.empty())
				fn = vm.currentScope->tryGenerateFunction(vm, fnName, { paramTypes.data(), numParams });
		}

		if (!fn) {
			std::string paramsString;
			for(size_t i = 0; i < numParams; ++i)
			{
				if (i > 0)
					paramsString += ", ";
				paramsString += paramTypes[i]->name();
			}

			throw std::runtime_error(fmt::format("Not matching function to call with params: {}.", paramsString));
		}

		if (fn->isConstructor)
		{
			constructor			= true;
			paramTypes[0]		= fn->outerType->shared_from_this();
			self				= vm.allocateReference(vm.allocateOnStack(paramTypes[0], nullptr));
			evaluatedArgs[0]	= *self;
			++numParams;
		}

		auto result = vm.executeFunction(*fn, Function::ArgSpan{ evaluatedArgs.data() + (self ? 0 : 1), numParams } );
		if (constructor)
			return evaluatedArgs[0];
		return result;
	}
	else if (op == "--" || op == "++")
	{
		auto lhs = *this->evalSingleAction(lhs_).as<RuntimeValue>();
		return executeIncrementDecrement(vm, op, lhs, Operator::Postfix);
	}

	return {};
}


}
