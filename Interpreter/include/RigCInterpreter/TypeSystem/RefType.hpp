#pragma once

#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/TypeSystem/IType.hpp>
#include <RigCInterpreter/TypeSystem/WrapperType.hpp>

namespace rigc::vm
{

struct RefType
	: WrapperType
{
	using WrapperType::WrapperType;

	std::string name() const override {
		return fmt::format("Ref<{}>", inner->name());
	}
	std::string symbolName() const override {
		return "Ref";
	}
	std::size_t size() const override {
		return sizeof(void*);
	}
	bool isArray() const override {
		return inner->isArray();
	}
	static std::size_t hashWrapped(InnerType const& inner_)
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

	std::string name() const override {
		return fmt::format("Addr<{}>", inner->name());
	}

	std::string symbolName() const override {
		return "Addr";
	}

	std::size_t size() const override {
		return sizeof(void*);
	}

	bool isArray() const override {
		return false;
	}

	static std::size_t hashWrapped(InnerType const& inner_)
	{
		return std::hash<std::string>{}(fmt::format("Addr<{}>", inner_->name()));
	}

	void postInitialize(Instance& vm_) override;
};


}
