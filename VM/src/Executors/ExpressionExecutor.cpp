#include "VM/include/RigCVM/RigCVMPCH.hpp"

#include <RigCVM/VM.hpp>
#include <RigCVM/Executors/ExpressionExecutor.hpp>

#include <RigCVM/TypeSystem/ClassType.hpp>
#include <RigCVM/TypeSystem/EnumType.hpp>
#include <RigCVM/TypeSystem/RefType.hpp>
#include <RigCVM/TypeSystem/FuncType.hpp>

#include <fmt/color.h>

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
auto isSymbol(rigc::ParserNode const& node_) -> bool
{
	return (
		node_.is_type<rigc::PossiblyTemplatedSymbol>() ||
		node_.is_type<rigc::PossiblyTemplatedSymbolNoDisamb>()
	);
}

////////////////////////////////////////
auto operatorPriority(rigc::ParserNode const& node_) -> int
{
	auto op = node_.string_view();
	if (op == ",") return 18;
	if (op == "as" || op == "as!") return 17;
	if (op == "=" || op == "+=" || op == "-=" || op == "*=" || op == "/=" || op == "%=")
		return 16;
	if (op == "or") return 15;
	if (op == "and") return 14;
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
		return vm.evaluate(*ctx.children.front());

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
			if ( !actions[i].is<PendingAction>() )
				continue;

			auto pa = actions[i].as<PendingAction>();

			if (isOperator(*pa))
			{
				auto priority = operatorPriority(*pa);
				if (priority < bestPriority)
				{
					bestIdx			= i;
					bestPriority	= priority;
				}
			}
		}

		this->evaluateAction(actions[bestIdx], bestIdx);

		numPending = rg::count_if(actions, &Action::is<PendingAction>);
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
			throw RigCError("Invalid infix operator position: {}", actionIndex_)
							.withLine(vm.lastEvaluatedLine);

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
			throw RigCError("Invalid prefix operator position: {}", actionIndex_)
							.withLine(vm.lastEvaluatedLine);

		action_ = this->evalPrefixOperator(
				oper.string_view(),
				actions[actionIndex_ + 1]
			);

		actions.erase(actions.begin() + actionIndex_ + 1);
	}
	else if (oper.is_type<rigc::PostfixOperator>())
	{
		if (actionIndex_ == 0)
			throw RigCError("Invalid postfix operator position: {}", actionIndex_)
							.withLine(vm.lastEvaluatedLine);

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

auto ExpressionExecutor::tryEvalDataMember(Value const& lhs, std::string_view memberName) -> OptValue
{
	if (auto classType = lhs.getClass())
	{
		if (auto member = classType->findDataMember(memberName))
		{
			return vm.allocateReference(lhs.member(member->offset, member->type));
		}
	}
	return std::nullopt;
}

////////////////////////////////////////
auto ExpressionExecutor::evalInfixOperator(StringView op_, Action& lhs_, Action& rhs_) -> ProcessedAction
{
	auto evalSide = [&](Action& side) -> Value { return *this->evalSingleAction(side).as<OptValue>(); };

	if (op_ == ".")
	{
		auto lhs = evalSide(lhs_);

		// TODO: ensure that the commented lines below are unnecessary
		// The reference was being removed right in the `isSymbol` section.
		// if (!lhs.type->is<RefType>())
		// 	lhs = vm.allocateReference(lhs);

		auto rhsExpr = rhs_.as<PendingAction>();

		/// Handle a symbol after the dot:
		///
		/// @example
		/// symbol.memberName
		/// (expr).memberName
		///
		/// @note
		/// "memberName" can be data member or a function name (possibly an extension method)
		if (isSymbol(*rhsExpr))
		{
			// Name is the first child of the (possibly templated) Symbol
			auto& nameNode = *rhsExpr->children[0];

			// TODO: ensure the reference is properly handled here
			lhs = lhs.safeRemoveRef();

			auto memberName = nameNode.string_view();

			if (auto val = this->tryEvalDataMember(lhs, memberName))
			{
				return val;
			}

			auto candidates = FunctionCandidates();

			// Try find type methods
			if (auto methods = lhs.type->findMethod(memberName))
			{
				// TODO: check the scope of the method
				candidates.push_back( { &vm.scopeOf(nullptr), methods } );
			}

			/// Try find free functions of the same name
			/// @note this is necessary for proper handling of extension methods.
			/// An extension method can be called like a method.
			///
			/// @example
			///
			/// func length(self: Vec2) -> Float32 { ... }
			/// var v = Vec2(1, 2);
			/// v.length(); // ✔ valid code
			/// @note 2: this will collect every function, not only extension methods.
			/// Non-extending methods will be filtered out during the `()` operator evaluation
			/// because it won't be a candidate.
			{
				auto freeFunctions = vm.findFunction(memberName);

				// TODO: properly handle the scope of the method.
				candidates.insert(std::end(candidates), std::begin(freeFunctions), std::end(freeFunctions));
			}

			return ProcessedFunction{ candidates, lhs, memberName };
		}
		else
		{
			throw RigCError("Invalid expression.")
							.withLine(vm.lastEvaluatedLine);
		}
	}
	else if(op_ == "::") // Handle the scope resultion operator
	{
		auto const rhs = rhs_.as<PendingAction>();
		if(!isSymbol(*rhs))
		{
			throw RigCError("Rhs of the :: operator should be a valid identifier.")
							.withHelp("Check the spelling of the right side of the operator.")
							.withLine(vm.lastEvaluatedLine);
		}


		auto const source = lhs_.as<PendingAction>();
		if(!isSymbol(*source))
		{
			throw RigCError("Lhs of the :: operator should be a valid identifier.")
							.withHelp("Check the spelling of the right side of the operator.")
							.withLine(vm.lastEvaluatedLine);
		}

		auto const memberName = rhs->string();
		auto const sourceName = source->string_view();

		auto const sourceType = vm.evaluateType(*source);

		if(!sourceType)
			throw RigCError("Type {} not found", sourceName)
							.withHelp("Check the spelling of the type.")
							.withLine(vm.lastEvaluatedLine);

		if (auto const enumType = sourceType->as<EnumType>())
		{
			if(enumType->fields.contains(memberName))
				return vm.allocateReference(enumType->fields.at(memberName));
			else
			 return {};
		}
		else
		{
			throw RigCError("Infix :: operator not implemented for anything else than enums for now.")
							.withHelp("Cope.")
							.withLine(vm.lastEvaluatedLine);
		}

	}
	else if(op_ == "as" || op_ == "as!")
	{
		auto lhs = evalSide(lhs_);

		auto const rhs = rhs_.as<PendingAction>();
		auto const rhsName = findElem<rigc::Name>(*rhs, false);

		if(!rhsName)
		{
			throw RigCError("Rhs of the conversion operator should be a valid identifier.")
							.withHelp("Check the spelling of the rhs.")
							.withLine(vm.lastEvaluatedLine);
		}

		auto const rhsType = vm.evaluateType(*rhs);
		if(!rhsType)
		{
			throw RigCError("Rhs of the conversion operator should be a type.")
							.withHelp("Check the spelling of the rhs and if the type is in scope.")
							.withLine(vm.lastEvaluatedLine);
		}

		auto lhsNoRef = lhs.safeRemoveRef();
		if (lhsNoRef.type->is<AddrType>() && rhsType->is<AddrType>())
		{
			if (lhsNoRef.type == rhsType)
			{
				auto& expr = lhs_.as<PendingAction>()->m_begin;
				fmt::print(fmt::fg(fmt::color::orange), "[{} L{}:C{}] Warning: converting from {} to {} is a no-op.\n",
						vm.currentModule->absolutePath.filename().string(),
						expr.line, expr.column,
						lhsNoRef.type->name(),
						rhsType->name()
					);
			}
			else if (op_ != "as!" && wrappedType(*rhsType).get() != vm.builtinTypes.Void)
			{
				throw RigCError("Conversion from {} to {} is not allowed.",
									lhsNoRef.type->name(),
									rhsType->name()
								)
								.withHelp("Use the danger conversion operator (\"as!\") instead.")
								.withLine(vm.lastEvaluatedLine);
			}

			return vm.allocateOnStack<void const*>(rhsType, lhsNoRef.view<void const*>());
		}

		return vm.tryConvert(lhs, rhsType);
	}
	else
	{
		auto lhs = evalSide(lhs_);
		auto rhs = evalSide(rhs_);

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

		throw RigCError("No matching operator \"{}\" found for argument types: ( {}, {} )", op_, lhs.type->name(), rhs.type->name())
						.withHelp("Try declaring that operator or check the types.")
						.withLine(vm.lastEvaluatedLine);
	}
}

auto tryGetFunctionArguments(rigc::ParserNode const& op_) -> Span<rigc::ParserNodePtr>
{
	if (auto args = findElem<rigc::ListOfFunctionArguments>(op_, false))
		return args->children;

	return {};
}

////////////////////////////////////////
size_t evaluateFunctionArguments(
		Instance&					vm_,
		Span<rigc::ParserNodePtr>	args_,
		Function::ArgSpan			evaluated_,
		FunctionParamTypeSpan		types_
	)
{
	size_t count = 0;
	for(auto& arg : args_)
	{
		evaluated_[count]	= vm_.evaluate(*arg).value();
		types_[count]		= evaluated_[count].type;
		++count;
	}
	return count;
}

auto ExpressionExecutor::tryFindEarlyBoundFunction(Action& action_, FunctionCandidates& candidates_) -> bool
{
	// Supports early binding if the symbol name is provided directly before `()` operator.
	// Valid code:
	//
	//    funcWithOverloads(param1, param2);
	//    func::< Int32, 32 >(param1, param2);
	//
	// Not matching example:
	//
	//    var func: Ref = funcWithOverloads; // 🔴 Error
	//    func(param1, param2);
	//
	auto expr = action_.as<PendingAction>();
	if (isSymbol(*expr))
	{
		// TODO: support function call template arguments
		// For now just ignore them and use the function name.
		auto symbolName = findElem<rigc::Name>(*expr)->string_view();

		candidates_ = vm.findFunction(symbolName);
		return true;
	}
	return false;
}

auto ExpressionExecutor::tryFindEarlyBoundMethod(Action& action_, FunctionCandidates& candidates_, OptValue& self_, StringView& functionName_) -> bool
{
	auto processedAction = action_.as<ProcessedAction>();
	if (processedAction.is<ProcessedFunction>())
	{
		/// This function was already processed for example by the evaluation of `.` operator,
		/// so just use the result here.
		///
		/// @example
		///
		/// symbol.func
		/// @see evalInfixOperator ('.' operator)

		auto& processedFunc = processedAction.as<ProcessedFunction>();

		candidates_		= processedFunc.candidates;
		self_			= processedFunc.self;
		functionName_	= processedFunc.name;

		return true;
	}

	return false;
}

void handleNoMatchingFunction(Instance& vm, StringView fnName, FunctionParamTypes const& paramTypes, size_t numParams)
{
	auto paramsString = String();
	auto paramNumber = size_t(0);
	for(size_t i = 0; paramNumber < numParams; ++i)
	{
		if (i == 0 && !paramTypes[i])
			continue;
		if (paramNumber > 0)
			paramsString += ", ";
		paramsString += paramTypes[i]->name();
		++paramNumber;
	}

	throw RigCError("Not matching function {}to call with arguments of type: {}.",
						fnName.empty() ? "" : fmt::format("\"{}\" ", fnName),
						paramsString
					)
					.withHelp("Check the function name and arguments' arity and types.")
					.withLine(vm.lastEvaluatedLine);
}

auto ExpressionExecutor::tryGenerateMethod(OptValue self, StringView fnName, Span< DeclType > reqParamTypes) -> Function const*
{
	if (self)
	{
		// Try generate method from method template inside the class type of `self`.
		if (auto classType = self->getClass())
		{
			return vm.scopeOf(classType->declaration).tryGenerateFunction(vm, fnName, reqParamTypes);
		}
	}
	return nullptr;
}

auto operatorName(rigc::ParserNode const& op_) -> StringView
{
	auto op = op_.string_view();

	if (op[0] == '(') return "()";
	else if (op[0] == '[') return "[]";
	else
		return op;
}

auto executeIncrementDecrement(Instance& vm, StringView op, Value& operand, Operator::Type operatorType) {
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

	throw RigCError(
		"No matching {} operator \"{}\" for argument type: {}.",
		operatorType == Operator::Type::Postfix ? "postfix" : "prefix",
		op,
		operand.type->name()
	)
	.withLine(vm.lastEvaluatedLine);
}

////////////////////////////////////////
auto ExpressionExecutor::evalPrefixOperator(StringView op_, Action& rhs_) -> ProcessedAction
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
		throw RigCError("Invalid prefix operator \"{}\".", op_)
						.withLine(vm.lastEvaluatedLine);

	return {};
}

////////////////////////////////////////
auto ExpressionExecutor::evalPostfixOperator(rigc::ParserNode const& op_, Action& lhs_) -> ProcessedAction
{
	auto op = op_.string_view();
	if (op[0] == '(' || op[0] == '[' || op == "--" || op == "++")
	{
		return this->evalGenericPostfixOperator(op_, lhs_);
	}
	else
	{
		throw RigCError("Invalid postfix operator \"{}\".", op)
				.withLine(vm.lastEvaluatedLine);
	}

	return {};
}


////////////////////////////////////////
auto ExpressionExecutor::evalGenericPostfixOperator(rigc::ParserNode const& op_, Action& lhs_) -> ProcessedAction
{
	auto candidates			= FunctionCandidates();
	auto self				= OptValue();
	auto opName				= operatorName(op_);
	auto supportOverloadRes	= false;

	auto ident				= lhs_.is<PendingAction>() ? findElem<rigc::Name>(*lhs_.as<PendingAction>()) : nullptr;
	auto identName			= ident ? ident->string_view() : StringView();
	auto identKind			= vm.getIdentifierType(identName);

	if (lhs_.is<PendingAction>())
	{
		// Function or TypeName because you can call type constructor using: TypeName()
		if ((identKind == Identifier::Function || identKind == Identifier::TypeName) && opName == "()")
		{
			if (this->tryFindEarlyBoundFunction(lhs_, candidates))
			{
				// Early binding was successful.
				// Multiple overloads could now be inside `candidates`.
				supportOverloadRes = true;
			}
		}
	}
	else if (this->tryFindEarlyBoundMethod(lhs_, candidates, self, identName))
	{
		supportOverloadRes = true;
	}

	if (!supportOverloadRes && identKind != Identifier::FunctionTemplate)
	{
		// There is still chance for early binding but with no overload resolution.
		self = lhs_.is<PendingAction>() ?
			this->evalSingleAction(lhs_).as<RuntimeValue>()
			:
			lhs_.as<ProcessedAction>().as<RuntimeValue>();

		if (auto ops = vm.universalScope().findOperator(opName, Operator::Postfix))
		{
			// fmt::print("Using the builtin operator{} found.\n", opName);
			candidates.push_back( { &vm.universalScope(), ops } );
		}
	}

	// First one is reserved to optional `self` (ignored if self is not provided)
	constexpr auto NonSelfParamStartIndex = size_t(1);

	Function::Args evaluatedArgs;
	FunctionParamTypes paramTypes;
	auto numParams = size_t(0);


	if (self)
	{
		// TODO: check why it works.
		evaluatedArgs[0] = supportOverloadRes ? vm.allocateReference(*self) : *self;

		paramTypes[0] = evaluatedArgs[0].type;
		++numParams;
	}

	auto argsNodeList = tryGetFunctionArguments(op_);

	auto paramCount = argsNodeList.empty() ? 0 : evaluateFunctionArguments(
			vm, argsNodeList,
			// Args span:
			viewArray(evaluatedArgs, NonSelfParamStartIndex),
			viewArray(paramTypes, NonSelfParamStartIndex)
		);
	numParams += paramCount;

	auto evalParamStartIdx = [&] { return (self ? 0 : NonSelfParamStartIndex); };

	auto reqParamTypes = viewArray(paramTypes, evalParamStartIdx(), numParams);

	auto fn = findOverload(candidates, reqParamTypes, self.has_value());

	if (!fn && !identName.empty())
	{
		fn = this->tryGenerateMethod(self, identName, reqParamTypes);

		// Try generate function from a global function template:
		if (!fn)
		{
			fn = vm.currentScope->tryGenerateFunction(vm, identName, reqParamTypes);
		}
	}

	if (!fn)
	{
		handleNoMatchingFunction(vm, identName, paramTypes, numParams);
	}

	// Create the object used by the constructor:
	// (Precondition: self was nullopt)
	if (fn->isConstructor)
	{
		paramTypes[0]		= fn->outerType->shared_from_this();
		self				= vm.allocateReference(vm.allocateOnStack(paramTypes[0], nullptr, 0));
		evaluatedArgs[0]	= *self;
		++numParams;
	}

	auto result = vm.executeFunction(*fn, viewArray( evaluatedArgs, evalParamStartIdx(), numParams) );

	if (fn->isConstructor)
	{
		return evaluatedArgs[0];
	}

	return result;
}


}
