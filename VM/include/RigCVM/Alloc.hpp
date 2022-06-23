#pragma once

#include <RigCVM/Value.hpp>
#include <RigCVM/TypeSystem/IType.hpp>

namespace rigc::vm {

//////////////////////////////////////////
template <typename T>
auto allocateStaticValue(DeclType type_, T const& value_) -> Value
{
	auto const data = new std::byte[ type_->size() ];
	new (data) T(value_);

	return Value { type_, data };
}

//////////////////////////////////////////
auto allocateStaticValue(DeclType type_, void const* sourceBytes_, size_t numBytes_ = 0) -> Value
{
	auto const data = new std::byte[ type_->size() ];

	auto toCopy = std::min(numBytes_, type_->size());
	std::memcpy(data, sourceBytes_, toCopy);

	return Value { type_, data };
}
}
