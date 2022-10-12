#pragma once

#include RIGCVM_PCH

#include <RigCVM/TypeSystem/TemplateParameter.hpp>

namespace rigc::vm
{

class ClassTemplate
{
public:
	ClassTemplate(String name_, DynArray<TemplateParameter> parameters_)
		: name(std::move(name_))
		, parameters(std::move(parameters_))
	{
	}

	String name;
	DynArray<TemplateParameter> parameters;

	auto instantiate(vm::Instance& vm_, DynArray<TemplateParameterValue> const& params_) const -> DeclType;
};
};
