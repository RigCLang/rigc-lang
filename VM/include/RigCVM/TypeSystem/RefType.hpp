#pragma once

#include RIGCVM_PCH

#include <RigCVM/TypeSystem/IType.hpp>
#include <RigCVM/TypeSystem/WrapperType.hpp>

namespace rigc::vm
{

struct RefType
	: WrapperType
{
	using WrapperType::WrapperType;

	auto name() const -> std::string override {
		return fmt::format("Ref<{}>", inner->name());
	}

	auto symbolName() const -> std::string override {
		return "Ref";
	}

	auto size() const -> std::size_t override {
		return sizeof(void*);
	}

	auto isArray() const -> bool override {
		return inner->isArray();
	}

	static auto hashWrapped(InnerType const& inner_) -> std::size_t
	{
		return std::hash<std::string>{}(fmt::format("Ref<{}>", inner_->name()));
	}
};

struct AddrType
	: WrapperType
{
	using WrapperType::WrapperType;

	AddrType(InnerType inner_ = nullptr)
		: WrapperType(std::move(inner_))
	{

	}

	auto name() const -> std::string override {
		return fmt::format("Addr<{}>", inner->name());
	}

	auto symbolName() const -> std::string override {
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
		return std::hash<std::string>{}(fmt::format("Addr<{}>", inner_->name()));
	}

	auto postInitialize(Instance& vm_) -> void override;
};


}
