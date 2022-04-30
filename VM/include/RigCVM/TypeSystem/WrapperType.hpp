#pragma once

#include RIGCVM_PCH

#include <RigCVM/TypeSystem/IType.hpp>
#include <RigCVM/Scope.hpp>

namespace rigc::vm
{

struct WrapperType
	: IType
{
	WrapperType(InnerType inner_ = nullptr)
		: inner(std::move(inner_))
	{}

	InnerType inner;
	auto decay() const -> InnerType override {
		return inner->decay();
	}
};

template <std::derived_from<WrapperType> Wrapper, typename... CtorTypes>
inline DeclType wrap(Scope& ownerScope_, DeclType decl, CtorTypes&&... ctorArgs)
{
	auto hash = Wrapper::hashWrapped(decl, std::as_const(ctorArgs)...);

	if (auto type = ownerScope_.types.find(hash))
		return type;

	auto wrapper = std::make_shared<Wrapper>(std::move(decl), std::forward<CtorTypes>(ctorArgs)...);
	ownerScope_.addType(wrapper);
	return wrapper;
}

}
