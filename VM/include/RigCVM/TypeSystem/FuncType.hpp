#pragma once

#include <RigCVM/RigCVMPCH.hpp>

#include <RigCVM/TypeSystem/IType.hpp>
#include <RigCVM/TypeSystem/TemplateType.hpp>
#include <RigCVM/Functions.hpp>

namespace rigc::vm
{


struct FuncType : TemplateType
{
	using Super = TemplateType;

	using Super::Super;

	auto name() const -> String override;
	auto symbolName() const -> String override {
		return "Func";
	}

	auto size() const -> std::size_t override {
		return sizeof(void*);
	}

	auto isArray() const -> bool override {
		return false;
	}

	auto decay() const -> InnerType override { return nullptr; }

	auto postInitialize(Instance& vm_) -> void override;

	static auto hashWrapped(DeclType const& returnType, Span<DeclType> args) -> std::size_t;
};

auto constructFunctionType(Scope& ownerScope_, Span<DeclType> args_) -> MutDeclType;


}
