#pragma once

#include RIGCVM_PCH

#include <RigCVM/TypeSystem/IType.hpp>
#include <RigCVM/TypeSystem/WrapperType.hpp>
#include <RigCVM/TypeSystem/TemplateParameter.hpp>

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

	std::vector<TemplateArgument> templateArguments;

	auto name() const -> std::string override {
		return fmt::format("StaticArray<{}, {}>", inner->name(), count);
	}

	auto symbolName() const -> std::string override
	{
		return "StaticArray";
	}

	auto size() const -> std::size_t override
	{
		return inner->size() * count;
	}

	auto isArray() const -> bool override
	{
		return true;
	}

	static auto hashWrapped(InnerType const& inner_, size_t count_) -> std::size_t
	{
		return std::hash<std::string>{}(fmt::format("StaticArray<{}, {}>", inner_->name(), count_));
	}

	auto getTemplateArguments() const -> std::vector<TemplateArgument> const& override;

	auto postInitialize(Instance& vm_) -> void override;
};



}
