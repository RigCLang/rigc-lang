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
struct EvaluatedParamList {
	Function::Params params;	
	std::size_t paramsCount;
};

////////////////////////////////////////
auto evaluateParamList(Instance& vm_, rigc::ParserNode const& expr_)
{
	auto params = Function::Params {{
		"self",
		wrap<RefType>(vm_.universalScope(), vm_.currentClass->shared_from_this())
	}};

	size_t numParams = 1;

	// TODO: refactor this
	auto paramList = findElem<rigc::FunctionParams>(expr_, false);
	if (paramList)
		evaluateFunctionParams(vm_, *paramList, params, numParams);

	return EvaluatedParamList{ params, numParams };
}

////////////////////////////////////////
auto evaluateOperatorDefinition(Instance &vm_, rigc::ParserNode const& expr_) -> OptValue
{
	auto& scope = vm_.scopeOf(vm_.currentClass->declaration);
	auto const& overloadedEntity = *findElem<rigc::OverloadedEntity>(expr_, false);

	auto const [params, paramsCount] = evaluateParamList(vm_, expr_);
	
	if(auto const entityName = findElem<rigc::Name>(overloadedEntity, false)) {
		auto const name = "conv " + entityName->string();

		auto& op =  scope.registerOperator(vm_, name, Operator::Infix, Function(Function::RuntimeFn(&expr_), params, paramsCount));
		op.returnsRef = returnsRef(expr_);
		vm_.currentClass->methods[name].push_back(&op);
		op.outerType = vm_.currentClass;
	}	
	else
		throw std::runtime_error("Only conversion operators supported for now.");

	return {};
}

}
