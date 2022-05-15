#pragma once

#include <RigCVM/Value.hpp>
#include <RigCVM/TypeSystem/IType.hpp>

namespace rigc::vm {

//////////////////////////////////////////
template <typename T>
auto allocateStaticValue(DeclType type_, T const& value_) -> Value {
	auto const size = type_->size();

	auto const data = new std::byte[size];
	new (data) T(value_);

	return Value { type_, data };
}

}
