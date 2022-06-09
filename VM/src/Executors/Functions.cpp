#include RIGCVM_PCH

#include <RigCVM/Executors/All.hpp>
#include <RigCVM/Executors/Templates.hpp>
#include <RigCVM/VM.hpp>

#include <RigCVM/TypeSystem/RefType.hpp>
#include <RigCVM/TypeSystem/ClassType.hpp>

namespace rigc::vm
{


////////////////////////////////////////
auto evaluateFunctionParams(Instance& vm_, rigc::ParserNode const& paramsNode_, Function::Params& params_, size_t& numParams_) -> void
{
	for (auto const& param : paramsNode_.children)
	{
		auto paramName	= findElem<rigc::Name>(*param)->string_view();
		auto type		= findElem<rigc::Type>(*param);

		params_[numParams_++] = { paramName, vm_.evaluateType(*type) };
	}
}


////////////////////////////////////////
auto evaluateFunctionDefinition(Instance &vm_, rigc::ParserNode const& expr_) -> OptValue
{
	auto& scope = *vm_.currentScope;

	auto name = findElem<rigc::Name>(expr_, false)->string_view();

	// auto const templateParamList = getTemplateParamList(expr_); 
	// std::pair<std::string, TypeConstraint>, string is a name,
	// TypeConstraint is, for now, a struct with just a name (std::string)
	// TODO: actually do something with the template parameter list

	bool returnsRef = false;
	if (auto explicitReturnType = findElem<rigc::ExplicitReturnType>(expr_, false))
	{
		auto type = findElem<rigc::Type>(*explicitReturnType)->string_view();
		if (type == "Ref")
			returnsRef = true;
	}

	Function::Params params;
	size_t numParams = 0;

	auto paramList = findElem<rigc::FunctionParams>(expr_, false);
	if (paramList)
		evaluateFunctionParams(vm_, *paramList, params, numParams);

	auto& func = scope.registerFunction(vm_, name, Function(Function::RuntimeFn(&expr_), params, numParams));
	func.returnsRef = returnsRef;

	return {};
}


////////////////////////////////////////
auto evaluateMethodDefinition(Instance &vm_, rigc::ParserNode const& expr_) -> OptValue
{
	auto& scope = vm_.scopeOf(vm_.currentClass->declaration);

	auto name = findElem<rigc::Name>(expr_, false)->string_view();

	bool returnsRef = false;
	if (auto explicitReturnType = findElem<rigc::ExplicitReturnType>(expr_, false))
	{
		auto type = findElem<rigc::Type>(*explicitReturnType)->string_view();
		if (type == "Ref")
			returnsRef = true;
	}

	Function::Params params;
	size_t numParams = 0;
	params[numParams++] = {
		"self",
		wrap<RefType>(vm_.universalScope(), vm_.currentClass->shared_from_this())
	};

	auto paramList = findElem<rigc::FunctionParams>(expr_, false);
	if (paramList)
		evaluateFunctionParams(vm_, *paramList, params, numParams);

	auto& method = scope.registerFunction(vm_, name, Function(Function::RuntimeFn(&expr_), params, numParams));
	method.returnsRef = returnsRef;
	vm_.currentClass->methods[name].push_back(&method);
	method.outerType = vm_.currentClass;

	return {};
}

////////////////////////////////////////
auto returnsRef(rigc::ParserNode const& expr_)
{
	if (auto explicitReturnType = findElem<rigc::ExplicitReturnType>(expr_, false))
	{
		auto type = findElem<rigc::Type>(*explicitReturnType)->string_view();
		if (type == "Ref")
			return true;
	}

	return false;
}


////////////////////////////////////////
auto getOperatorQualifier(rigc::ParserNode const& expr_) -> std::optional<Operator::Type>
{
	if(auto const preQualifier = findElem<rigc::PreKeyword>(expr_, false))
		return Operator::Type::Prefix;
	else if(auto const postQualifier = findElem<rigc::PostKeyword>(expr_, false))
		return Operator::Type::Postfix;

	return {};
}

auto evaluateFreeOperatorDefinition(Instance& vm_, Scope& scope_, rigc::ParserNode const& expr_, Function& func_, Operator op_) 
{
	auto params = Function::Params {};
	auto paramsCount = size_t(0);

	auto paramList = findElem<rigc::FunctionParams>(expr_, false);
	if (paramList)
		evaluateFunctionParams(vm_, *paramList, params, paramsCount);

	func_.params = std::move(params);
	func_.paramCount = paramsCount;
	auto& registeredOp = scope_.registerOperator(vm_, op_.str, op_.type, std::move(func_));
	registeredOp.returnsRef = returnsRef(expr_);
}

auto evaluateMemberOperatorDefinition(Instance& vm_, Scope& scope_, rigc::ParserNode const& expr_, Function& func_, Operator op_)
{
	auto params = Function::Params {
		"self",
		wrap<RefType>( vm_.universalScope(), vm_.currentClass->shared_from_this() )
	};
	auto paramsCount = size_t(1);

	auto paramList = findElem<rigc::FunctionParams>(expr_, false);
	if (paramList)
		evaluateFunctionParams(vm_, *paramList, params, paramsCount);

	func_.params = std::move(params);
	func_.paramCount = paramsCount;
	auto& registeredOp = scope_.registerOperator(vm_, op_.str, op_.type, std::move(func_));
	registeredOp.returnsRef = returnsRef(expr_);
	registeredOp.outerType = vm_.currentClass;

	vm_.currentClass->methods[op_.str].push_back(&registeredOp);
}

auto evaluateOverloadedEntity(Instance& vm_, ParserNode const& expr_) -> Operator
{
	auto const& overloadedEntity = *findElem<rigc::OverloadedEntity>(expr_, false);
	auto op = Operator();

	if(auto const entityName = findElem<rigc::Name>(overloadedEntity, false)) // conversion operator
	{
		auto const conversionType = vm_.findType(entityName->string_view());
		if(!conversionType)
			throw std::runtime_error("Cannot declare a conversion operator for a type that doesn't exist.");
	
		op = { "convert", Operator::Type::Infix };
	}	
	else if(auto const postfixOperator = findElem<rigc::PostfixOperator>(overloadedEntity, false)) // ++ and -- fall under this category
	{
		op.str = postfixOperator->string_view();
		auto const opQualifier = getOperatorQualifier(expr_);

		if(op.str != "++" || op.str != "--")
		{
			if(opQualifier)
				throw std::runtime_error("Cannot use 'pre' operator qualifier for a postfix operator.");

			op.type = Operator::Type::Postfix;
		}
		else
		{
			if(!opQualifier)
				throw std::runtime_error("No operator qualifier given for an ambiguous operator.");

			op.type = *opQualifier;
		}
	}
	else if(auto const prefixOperator = findElem<rigc::PrefixOperator>(overloadedEntity, false))
		op = { prefixOperator->string_view(), Operator::Type::Prefix };

	else
		op = { overloadedEntity.string_view(), Operator::Type::Infix };


	return op;
}

////////////////////////////////////////
auto evaluateOperatorDefinition(Instance &vm_, rigc::ParserNode const& expr_) -> OptValue
{
	auto const overloadedEntity = findElem<rigc::OverloadedEntity>(expr_, false);
	auto func = Function(Function::RuntimeFn(&expr_), {}, 0);
	auto op = evaluateOverloadedEntity(vm_, expr_);

	if(vm_.currentClass)
	{
		evaluateMemberOperatorDefinition(vm_, vm_.scopeOf(vm_.currentClass->declaration), expr_, func, op);
	}
	else
	{
		evaluateFreeOperatorDefinition(vm_, vm_.universalScope(), expr_, func, op);
	}

	return {};
}

}
