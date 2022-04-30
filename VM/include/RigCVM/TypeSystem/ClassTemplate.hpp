#pragma once

#include RIGCVM_PCH

#include <RigCVM/TypeSystem/TemplateParameter.hpp>

namespace rigc::vm
{

class ClassTemplate
{
public:
	ClassTemplate(std::string name_, Vec<TemplateParameter> parameters_)
		: name(std::move(name_))
		, parameters(std::move(parameters_))
	{
	}

	std::string name;
	Vec<TemplateParameter> parameters;

	auto instantiate(vm::Instance& vm_, Vec<TemplateParameterValue> const& params_) const -> DeclType;
};

};
