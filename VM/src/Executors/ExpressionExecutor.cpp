#include RIGCVM_PCH

#include <tuple>

#include <RigCVM/VM.hpp>
#include <RigCVM/Executors/ExpressionExecutor.hpp>

#include <RigCVM/TypeSystem/ClassType.hpp>
#include <RigCVM/TypeSystem/EnumType.hpp>
#include <RigCVM/TypeSystem/RefType.hpp>
#include <RigCVM/TypeSystem/FuncType.hpp>
#include <RigCVM/TypeSystem/WrapperType.hpp>

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

auto getOperatorOverloads(Instance& vm_, DeclType const& type_, Operator const& operator_, bool variable = false) -> std::pair<FunctionOverloads const*, bool> {
	if(auto const& refType = type_->as<RefType>(); refType && refType->inner->is<ClassType>())
	{
		return { &refType->inner->methods[operator_.str], true };
	}
	else if(auto const& classType = type_->as<ClassType>())
	{
		return { &classType->methods[operator_.str], true };
	}

	return { vm_.currentScope->findOperatorGlobally(operator_.str, operator_.type), false };
}

////////////////////////////////////////
auto ExpressionExecutor::evalInfixOperatorNonOverloadable(std::string_view op_, Action& lhs_, Action& rhs_) -> OptValue
{
	auto lhs	= *this->evalSingleAction(lhs_);

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

	return {};
}


////////////////////////////////////////
auto ExpressionExecutor::evalInfixOperator(std::string_view op_, Action& lhs_, Action& rhs_) -> OptValue
{
	if(op_ == "." || op_ == "::")
		return evalInfixOperatorNonOverloadable(op_, lhs_, rhs_);

	auto const lhs	= this->evalSingleAction(lhs_)->safeRemovePtr();
	auto params = FunctionParamTypes();
	auto args = Function::Args();
	auto argsCount = 0;

	Function::ReturnType returnType;

	if(op_ == "as")
	{
		op_ = "convert";
		auto const rhs = rhs_.as<PendingAction>();
		if(!rhs->is_type<rigc::Name>())
			throw std::runtime_error("Rhs of the conversion operator should be a valid identifier.");

		auto const rhsType = vm.findType(rhs->string_view());
		if(!rhsType)
			throw std::runtime_error("Rhs of the conversion identifier should be a type.");

		if(lhs.type->is<CoreType>())
			return vm.tryConvert(lhs.safeRemoveRef(), rhsType->shared_from_this());

		params[argsCount] = wrap<RefType>( vm.universalScope(), lhs.type );
		args[argsCount++] = lhs;
		returnType = rhsType->shared_from_this();
	}
	else
	{
		auto const rhs = *this->evalSingleAction(rhs_);

		auto const lhsType = lhs.type->is<RefType>()
			? lhs.type
			: wrap<RefType>( vm.universalScope(), lhs.type );

		params[argsCount] = lhsType;
		args[argsCount++] = lhs;
		params[argsCount] = rhs.type;
		args[argsCount++] = rhs;
	}

	const auto[overloads, isMethod] = getOperatorOverloads(vm, lhs.type, { op_, Operator::Infix });
	if(!overloads)
		throw std::runtime_error(fmt::format("Cannot find any {} operator for types: {}", op_, lhs.typeName()));

	const auto fittingOv = findOverload(*overloads, params, argsCount, isMethod, returnType);
	if(!fittingOv)
		throw std::runtime_error(fmt::format("No fitting overload for op {} and type {}.", op_, lhs.typeName()));

	return vm.executeFunction(*fittingOv, args, 1);
}

////////////////////////////////////////

////////////////////////////////////////
auto ExpressionExecutor::evalPrefixOperator(std::string_view op_, Action& rhs_) -> OptValue
{
	auto rhs = *this->evalSingleAction(rhs_);

	auto const evalDereferenceOrAddressTake = [this, op_, &rhs] {
		if(op_ == "&") return vm.allocatePointer(rhs);
		else return rhs.safeRemoveRef().removePtr();
	};

	auto params = FunctionParamTypes();
	auto args = Function::Args();
	auto argsCount = 0;

	if (op_ == "*" || op_ == "&" || op_ == "--" || op_ == "++")
	{
		params[0] = rhs.type;
		args[0] = rhs;
		argsCount = 1;
	}
	else
		throw std::runtime_error("Invalid prefix operator: " + std::string(op_));

	const auto[overloads, isMethod] = getOperatorOverloads(vm, rhs.type, { op_, Operator::Prefix });
	if(!overloads) 
	{
		if(op_ == "&" || op_ == "*") return evalDereferenceOrAddressTake();
		throw std::runtime_error(fmt::format("Cannot find prefix operator {} for type: {}", op_, rhs.typeName()));
	}

	const auto fittingOv = findOverload(*overloads, params, 1, isMethod);
	if(!fittingOv)
	{
		if(op_ == "&" || op_ == "*") return evalDereferenceOrAddressTake();
		throw std::runtime_error(fmt::format("No fitting overload for op {} and type {}.", op_, rhs.typeName()));
	}

	return vm.executeFunction(*fittingOv, args, 1);
}

////////////////////////////////////////

////////////////////////////////////////
auto evaluateArgumentList(Instance& vm_, rigc::ParserNode const& list_, std::size_t numParams = 0)
{
	auto paramTypes = FunctionParamTypes();
	auto args = Function::Args();

	for(size_t i = 0; i < list_.children.size(); ++i)
	{
		args[numParams]	= vm_.evaluate(*list_.children[i]).value();
		paramTypes[numParams]		= args[numParams].type;
		numParams++;
	}

	return std::tuple( std::move(paramTypes), std::move(args), numParams );
}

////////////////////////////////////////
auto prepareSelfArgument(Instance& vm_, DeclType const& selfType, Value const& val_, FunctionParamTypes& params_, Function::Args& args_)
{
	params_[0] = wrap<RefType>(vm_.universalScope(), selfType);
	args_[0]	= vm_.allocateReference(val_);
}

////////////////////////////////////////
auto ExpressionExecutor::evalFunctionCallOperator(rigc::ParserNode const& op_, Action& lhs_) -> OptValue
{
	auto lhsVal = this->evalSingleAction(lhs_)->safeRemoveRef();
	auto const& argsList = *findElem<rigc::ListOfFunctionArguments>(op_, false);

	// If it's not a function type then it needs a self argument (method, function call op overload, constructor),
	// so we start filling from the second one
	auto const startIndex = lhsVal.type->is<FuncType>() ? 0 : 1;
	auto [params, args, numArgs] = evaluateArgumentList(vm, argsList, startIndex);

	auto overloads = lhsVal.view<FunctionOverloads const*>();

	auto isMethod = false;

	if(lhs_.is<ProcessedAction>()) // method since obj.methodnName has already been evaluated
	{
		isMethod = true;
		prepareSelfArgument(vm, lhsVal.type->as<MethodType>()->classType, lhsVal, params, args);
	}
	else
	{
		auto const calledEntityName = lhs_.as<PendingAction>()->string_view();

		if(auto type = vm.findType(calledEntityName))
		{
			// TODO: type construction doesnt work yet
			if(const auto classType = type->as<ClassType>())
				throw std::runtime_error("Type construction has not been implemented yet.");
			else if(const auto enumType = type->as<EnumType>())
				return vm.allocateOnStack(enumType->shared_from_this(), Value { enumType->shared_from_this(), nullptr });
			else if(const auto coreType = type->as<CoreType>())
				return vm.allocateOnStack(coreType->shared_from_this(), lhsVal.blob());
			else
				throw std::runtime_error("Trying to construct a non class type.");
		}
		else if(const auto var = vm.findVariableByName(calledEntityName))
		{
			overloads = getOperatorOverloads(vm, var->type, { "()", Operator::Postfix }, true).first;
			prepareSelfArgument(vm, var->type, *var, params, args);
			isMethod = true;
		}
	}
	if(!overloads)
		throw std::runtime_error(fmt::format("Cannot find function call operator for type: {}", lhsVal.type->symbolName()));

	const auto fn = findOverload(*overloads, params, numArgs, isMethod);
	if(!fn)
		throw std::runtime_error(fmt::format("No valid overloads for type: {}", lhsVal.typeName()));

	return vm.executeFunction(*fn, args, numArgs);
}

////////////////////////////////////////
auto ExpressionExecutor::evalPostfixOperator(rigc::ParserNode const& op_, Action& lhs_) -> OptValue
{
	auto lhs = *this->evalSingleAction(lhs_);
	auto op = op_.string_view();

	if(op[0] == '(') return evalFunctionCallOperator(op_, lhs_);

	auto params = FunctionParamTypes();
	auto args = Function::Args();
	auto argsCount = 0;

	Function::ReturnType returnType;

	if (op[0] == '[') // array access
	{
		op = "[]";
		auto const expr = findElem<rigc::Expression>(op_, false);
		if(!expr)
			throw std::runtime_error("Expression expected inside [].");

		auto const indexExpr = vm.evaluate(*expr);
		if(!indexExpr)
			throw std::runtime_error("Invalid expression inside [].");

		if(!lhs.safeRemoveRef().type->is<ClassType>())
		{
			auto const data = lhs.safeRemoveRef();
			auto const elemType	= data.type->decay();

			return vm.allocateReference(Value{
				elemType,
				(char*)data.data + indexExpr->view<int>() * elemType->size()
			});
		}

		prepareSelfArgument(vm, lhs.type, lhs, params, args);
		params[1] = indexExpr->type;
		args[1] = *indexExpr;
		argsCount = 2;
	}
	else if (op == "--" || op == "++")
	{
		params[0] = lhs.type;
		args[0] = lhs;
		argsCount = 1;
	}

	const auto[overloads, isMethod] = getOperatorOverloads(vm, lhs.type, { op, Operator::Postfix });
	if(!overloads)
		throw std::runtime_error(fmt::format("Cannot find postfix operator {} for type: {}", op, lhs.type->symbolName()));

	const auto fittingOv = findOverload(*overloads, params, argsCount, isMethod, returnType);
	if(!fittingOv)
		throw std::runtime_error(fmt::format("No fitting overload for op {} and type {}.", op, lhs.typeName()));

	return vm.executeFunction(*fittingOv, args, argsCount);
}


}
