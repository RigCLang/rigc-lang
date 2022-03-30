#pragma once

#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/TypeSystem/IType.hpp>
#include <RigCInterpreter/TypeSystem/WrapperType.hpp>
#include <RigCInterpreter/TypeSystem/TemplateParameter.hpp>

namespace rigc::vm
{

struct ArrayType
	: WrapperType
{
	ArrayType() = default;
	ArrayType(InnerType inner_, size_t size_)
		:
		WrapperType( std::move(inner_) ),
		count{ size_ }
	{}

	size_t count = 1;

	std::vector<TemplateParameter> templateParams;

	std::string name() const override {
		return fmt::format("Array<{}, {}>", inner->name(), count);
	}
	std::string symbolName() const override {
		return "Array";
	}
	std::size_t size() const override {
		return inner->size() * count;
	}
	bool isArray() const override {
		return true;
	}
	static std::size_t hashWrapped(InnerType const& inner_, size_t count_)
	{
		return std::hash<std::string>{}(fmt::format("Array<{}, {}>", inner_->name(), count_));
	}
};



}
