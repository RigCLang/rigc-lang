#include RIGCVM_PCH

#include <RigCVM/Executors/All.hpp>
#include <RigCVM/Executors/Templates.hpp>
#include <RigCVM/VM.hpp>

#include <RigCVM/TypeSystem/RefType.hpp>
#include <RigCVM/TypeSystem/ClassType.hpp>

namespace rigc::vm
{
////////////////////////////////////////
auto isTemplatedType(rigc::ParserNode const& typeNode_, TemplateParameters const& templateParams_) -> bool
{
	auto templParamsNode = findElem<rigc::TemplateParams>(typeNode_);

	if (!templParamsNode)
	{
		return templateParams_.find(typeNode_.string_view()) != templateParams_.end();
	}

	for (auto const& t : templParamsNode->children)
	{
		// fmt::print(" $ testing {} (type: {})\n", t->string_view(), t->type);
		if (isTemplatedType(*t, templateParams_))
			return true;
	}
	return false;
}

////////////////////////////////////////
auto evaluateFunctionParams(Instance& vm_, rigc::ParserNode const& paramsNode_, Function::Params& params_,
						    size_t& numParams_, TemplateParameters& templateParams_) -> void
{
	for (auto const& param : paramsNode_.children)
	{
		auto paramName	= findElem<rigc::Name>(*param)->string_view();
		auto type		= findElem<rigc::Type>(*param);

		if (isTemplatedType(*type, templateParams_))
			params_[numParams_] = { paramName, nullptr, type }; // Later evaluation
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

	auto templateParams = TemplateParameters();
	for (auto& tp : templateParamList)
	{
		templateParams[tp.first] = tp.second;
		// fmt::print("{} is constrained with {}\n", tp.first, tp.second.name);
	}

	// TODO: Properly parse return type
	bool returnsRef = false;
	auto returnType = DeclType();
	if (auto explicitReturnType = findElem<rigc::ExplicitReturnType>(expr_, false))
	{
		if (!isTemplatedType(*findElem<rigc::Type>(*explicitReturnType), templateParams))
		{
			returnType = vm_.evaluateType(*findElem<rigc::Type>(*explicitReturnType));
			if (returnType->is<RefType>())
				returnsRef = true;
		}
		// else: Leave the return type empty -> it has to be resolved for each instantiation
	}
	else
		returnType = vm_.builtinTypes.Void.shared();

	Function::Params params;
	size_t numParams = 0;

	auto paramList = findElem<rigc::FunctionParams>(expr_, false);
	if (paramList)
		evaluateFunctionParams(vm_, *paramList, params, numParams, templateParams);

	Function* func = nullptr;
	if (isTemplate)
		func = &scope.registerFunctionTemplate(vm_, name, Function(Function::RuntimeFn(&expr_), params, numParams));
	else
		func = &scope.registerFunction(vm_, name, Function(Function::RuntimeFn(&expr_), params, numParams));

	func->returnsRef = returnsRef;
	func->returnType = std::move(returnType);
	auto& funcScope = vm_.scopeOf(func);
	funcScope.templateParams = std::move(templateParams);


	return {};
}

////////////////////////////////////////
auto evaluateMethodDefinition(Instance &vm_, rigc::ParserNode const& expr_) -> OptValue
{
	auto& scope = vm_.scopeOf(vm_.currentClass->declaration);

	auto name = findElem<rigc::Name>(expr_, false)->string_view();

	auto templateParamList = getTemplateParamList(expr_);
	auto isTemplate = !templateParamList.empty();

	auto templateParams = TemplateParameters();
	for (auto& tp : templateParamList)
	{
		templateParams[tp.first] = tp.second;
		// fmt::print("{} is constrained with {}\n", tp.first, tp.second.name);
	}


	// TODO: Properly parse return type
	bool returnsRef = false;
	auto returnType = DeclType();
	if (auto explicitReturnType = findElem<rigc::ExplicitReturnType>(expr_, false))
	{
		if (!isTemplatedType(*findElem<rigc::Type>(*explicitReturnType), templateParams))
		{
			returnType = vm_.evaluateType(*findElem<rigc::Type>(*explicitReturnType));
			if (returnType->is<RefType>())
				returnsRef = true;
		}
		// else: Leave the return type empty -> it has to be resolved for each instantiation
	}
	else
		returnType = vm_.builtinTypes.Void.shared();

	Function::Params params;
	size_t numParams = 0;
	params[numParams++] = {
		"self",
		constructTemplateType<RefType>(vm_.universalScope(), vm_.currentClass->shared_from_this())
	};

	auto paramList = findElem<rigc::FunctionParams>(expr_, false);
	if (paramList)
		evaluateFunctionParams(vm_, *paramList, params, numParams, templateParams);

	Function* method = nullptr;
	if (isTemplate)
		method = &scope.registerFunctionTemplate(vm_, name, Function(Function::RuntimeFn(&expr_), params, numParams));
	else
		method = &scope.registerFunction(vm_, name, Function(Function::RuntimeFn(&expr_), params, numParams));

	method->returnsRef = returnsRef;
	method->returnType = std::move(returnType);
	vm_.currentClass->methods[name].push_back(method);
	method->outerType = vm_.currentClass;

	auto& methodScope = vm_.scopeOf(method);
	methodScope.templateParams = std::move(templateParams);

	if (name == "construct")
		method->isConstructor = true;

	return {};
}
}
