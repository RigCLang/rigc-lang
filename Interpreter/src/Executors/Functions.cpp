#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Executors/All.hpp>
#include <RigCInterpreter/Executors/ExpressionExecutor.hpp>
#include <RigCInterpreter/VM.hpp>

#include <RigCInterpreter/TypeSystem/ArrayType.hpp>
#include <RigCInterpreter/TypeSystem/RefType.hpp>
#include <RigCInterpreter/TypeSystem/ClassType.hpp>

namespace rigc::vm
{


////////////////////////////////////////
void evaluateFunctionParams(Instance& vm_, rigc::ParserNode const& paramsNode_, Function::Params& params_, size_t& numParams_)
{
	for (auto const& param : paramsNode_.children)
	{
		auto paramName	= findElem<rigc::Name>(*param)->string_view();
		auto type		= findElem<rigc::Type>(*param);

		params_[numParams_++] = { paramName, vm_.evaluateType(*type) };
	}
}

////////////////////////////////////////
OptValue evaluateFunctionDefinition(Instance &vm_, rigc::ParserNode const& expr_)
{
	auto& scope = *vm_.currentScope;

	auto name = findElem<rigc::Name>(expr_, false)->string_view();

	Function::Params params;
	size_t numParams = 0;

	auto paramList = findElem<rigc::FunctionParams>(expr_, false);
	if (paramList)
		evaluateFunctionParams(vm_, *paramList, params, numParams);

	scope.registerFunction(vm_, name, Function(Function::RuntimeFn(&expr_), params, numParams));

	return {};
}


////////////////////////////////////////
OptValue evaluateMethodDefinition(Instance &vm_, rigc::ParserNode const& expr_)
{
	auto& scope = vm_.scopeOf(vm_.currentClass->declaration);

	auto name = findElem<rigc::Name>(expr_, false)->string_view();

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
	vm_.currentClass->methods[name].push_back(&method);
	method.outerType = vm_.currentClass;

	return {};
}


}
