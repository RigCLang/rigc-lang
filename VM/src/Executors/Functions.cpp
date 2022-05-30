#include RIGCVM_PCH

#include <RigCVM/Executors/All.hpp>
#include <RigCVM/Executors/Templates.hpp>
#include <RigCVM/VM.hpp>

#include <RigCVM/TypeSystem/RefType.hpp>
#include <RigCVM/TypeSystem/ClassType.hpp>

namespace rigc::vm
{


////////////////////////////////////////
auto evaluateFunctionParams(Instance& vm_, rigc::ParserNode const& paramsNode_, Function::Params& params_, size_t& numParams_, Scope& funcScope_) -> void
{
	auto& templateParams = funcScope_.templateParams;

	for (auto const& param : paramsNode_.children)
	{
		auto paramName	= findElem<rigc::Name>(*param)->string_view();
		auto type		= findElem<rigc::Type>(*param);

		auto templateParamIt = templateParams.find(type->string_view());
		if (templateParamIt != templateParams.end())
			params_[numParams_] = { paramName, nullptr, type->string_view() }; // Later evaluation
		else
			params_[numParams_] = { paramName, vm_.evaluateType(*type) };

		numParams_++;
	}
}


////////////////////////////////////////
auto evaluateFunctionDefinition(Instance &vm_, rigc::ParserNode const& expr_) -> OptValue
{
	auto& scope = *vm_.currentScope;

	auto name = findElem<rigc::Name>(expr_, false)->string_view();

	auto templateParamList = getTemplateParamList(expr_);
	auto isTemplate = !templateParamList.empty();

	auto& funcScope = vm_.scopeOf(&expr_);
	for (auto& tp : templateParamList)
	{
		funcScope.templateParams[tp.first] = tp.second;
		// fmt::print("{} is constrained with {}\n", tp.first, tp.second.name);
	}

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
		evaluateFunctionParams(vm_, *paramList, params, numParams, funcScope);

	if (isTemplate)
	{
		auto& func = scope.registerFunctionTemplate(vm_, name, Function(Function::RuntimeFn(&expr_), params, numParams));
		func.returnsRef = returnsRef;
	}
	else
	{
		auto& func = scope.registerFunction(vm_, name, Function(Function::RuntimeFn(&expr_), params, numParams));
		func.returnsRef = returnsRef;
	}

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
		evaluateFunctionParams(vm_, *paramList, params, numParams, vm_.scopeOf(&expr_));

	auto& method = scope.registerFunction(vm_, name, Function(Function::RuntimeFn(&expr_), params, numParams));
	method.returnsRef = returnsRef;
	vm_.currentClass->methods[name].push_back(&method);
	method.outerType = vm_.currentClass;

	if (name == "construct")
		method.isConstructor = true;

	return {};
}


}
