#pragma once

#include RIGCVM_PCH

#include <RigCVM/TypeSystem/IType.hpp>
#include <RigCVM/TypeSystem/WrapperType.hpp>
#include <RigCVM/TypeSystem/TemplateParameter.hpp>

namespace rigc::vm
{

struct ArrayType : TemplateType
{
	using Super = TemplateType;

	ArrayType() = default;
	ArrayType(InnerType inner_, size_t size_)
		:
		TemplateType( std::move(inner_) )
	{
		// TODO: support types other than int as NTTPs
		args.push_back( int(size_) );
	}

	auto inner() const { return args.front().as<DeclType>(); }

	auto name() const -> String override {
		return fmt::format("StaticArray<{}, {}>", this->inner()->name(), this->count());
	}

	auto symbolName() const -> String override
	{
		return "StaticArray";
	}

	auto count() const -> std::size_t
	{
		return args[1].as<int>();
	}

	auto size() const -> std::size_t override
	{
		return this->inner()->size() * args[1].as<int>();
	}

	auto isArray() const -> bool override
	{
		return true;
	}

	static auto hashWrapped(InnerType const& inner_, size_t count_) -> std::size_t
	{
		return std::hash<String>{}(fmt::format("StaticArray<{}, {}>", inner_->name(), count_));
	}

	auto postInitialize(Instance& vm_) -> void override;
};
}
