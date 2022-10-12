#pragma once

#include RIGCVM_PCH

#include <RigCVM/TypeSystem/IType.hpp>
#include <RigCVM/TypeSystem/WrapperType.hpp>

namespace rigc::vm
{
struct RefType : TemplateType
{
	using Super = TemplateType;

	using Super::Super;

	auto inner() const -> DeclType { return args.front().as<DeclType>(); }

	auto name() const -> String override {
		return fmt::format("Ref<{}>", this->inner()->name());
	}

	auto symbolName() const -> String override {
		return "Ref";
	}

	auto size() const -> std::size_t override {
		return sizeof(void*);
	}

	auto isArray() const -> bool override {
		return this->inner()->isArray();
	}

	static auto hashWrapped(InnerType const& inner_) -> std::size_t
	{
		return std::hash<String>{}(fmt::format("Ref<{}>", inner_->name()));
	}

	auto postInitialize(Instance& vm_) -> void override;
};

struct AddrType : TemplateType
{
	using Super = TemplateType;

	using Super::Super;

	auto inner() const { return args.front().as<DeclType>(); }

	auto name() const -> String override {
		return fmt::format("Addr<{}>", this->inner()->name());
	}

	auto symbolName() const -> String override {
		return "Addr";
	}

	auto size() const -> std::size_t override {
		return sizeof(void*);
	}

	auto isArray() const -> bool override {
		return false;
	}

	static auto hashWrapped(InnerType const& inner_) -> std::size_t
	{
		return std::hash<String>{}(fmt::format("Addr<{}>", inner_->name()));
	}

	auto postInitialize(Instance& vm_) -> void override;
};
}
