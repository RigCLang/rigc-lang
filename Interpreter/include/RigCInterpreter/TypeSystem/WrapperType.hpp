#pragma once

#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/TypeSystem/IType.hpp>
#include <RigCInterpreter/TypeSystem/TypeRegistry.hpp>

namespace rigc::vm
{

struct WrapperType
	: IType
{
	WrapperType(InnerType inner_ = nullptr)
		: inner(std::move(inner_))
	{}

	InnerType inner;
	InnerType decay() const override {
		return inner->decay();
	}
};

template <typename Wrapper, typename... CtorTypes>
inline DeclType wrap(TypeRegistry& reg_, DeclType decl, CtorTypes&&... ctorArgs)
{
	auto hash = Wrapper::hashWrapped(decl, std::as_const(ctorArgs)...);

	if (auto type = reg_.find(hash))
		return type;

	auto wrapper = std::make_shared<Wrapper>(std::move(decl), std::forward<CtorTypes>(ctorArgs)...);
	reg_.add(wrapper);
	return wrapper;
}

}
