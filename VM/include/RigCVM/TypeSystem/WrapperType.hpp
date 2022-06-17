#pragma once

#include RIGCVM_PCH

#include <RigCVM/TypeSystem/IType.hpp>
#include <RigCVM/Scope.hpp>

namespace rigc::vm
{

struct TemplateType
	: IType
{
	Vec<TemplateArgument> args;

	TemplateType()
	{}

	// DEPRECATED
	TemplateType(DeclType inner_)
		: args{ std::move(inner_) }
	{}

	TemplateType(std::span<DeclType> args_)
		: args(args_.begin(), args_.end())
	{}

	auto decay() const -> InnerType override {
		return args.front().as<DeclType>()->decay();
	}

	auto getTemplateArguments() const -> std::vector<TemplateArgument> const& override
	{
		return args;
	}
};

template <std::derived_from<TemplateType> Wrapper, typename... CtorTypes>
inline MutDeclType constructTemplateType(Scope& ownerScope_, DeclType decl, CtorTypes&&... ctorArgs)
{
	auto hash = Wrapper::hashWrapped(decl, std::as_const(ctorArgs)...);

	if (auto type = ownerScope_.types.find(hash))
		return type;

	auto wrapper = std::make_shared<Wrapper>(std::move(decl), std::forward<CtorTypes>(ctorArgs)...);
	ownerScope_.addType(wrapper);
	return wrapper;
}

}
