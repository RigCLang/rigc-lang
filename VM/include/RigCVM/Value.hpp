#pragma once

#include RIGCVM_PCH

#include <RigCVM/Type.hpp>
#include <RigCVM/Stack.hpp>

namespace rigc::vm
{
struct Instance;
struct DataMember;

template <typename T>
using Ptr = T*;
struct ValueBase
{
	DeclType	type;

	auto getType() const -> DeclType const& {
		return type;
	}

	auto typeName() const -> std::string {
		auto decayed = type->decay();
		return (decayed ? decayed.get() : type.get())->name();
	}
};

struct Value : ValueBase
{
	void* data = nullptr;

	template <typename T>
	auto view() -> T& 
	{
		return *reinterpret_cast<T*>(data);
	}

	template <typename T>
	auto view() const -> T const& 
	{
		return *reinterpret_cast<T*>(data);
	}

	// Temp:
	auto blob() const -> void* 
	{
		return data;
	}

	auto member(DataMember const& dm_) const-> Value;

	auto member(size_t offset_, DeclType type_) const-> Value;

	auto safeRemoveRef() const-> Value;
	auto safeRemovePtr() const-> Value;
	auto removeRef() const-> Value;
	auto removePtr() const-> Value;
};

auto dump(Instance& vm_, Value const& value_)-> std::string;

struct CompileTimeValue : ValueBase
{
	std::vector<std::byte> buffer;

	template <typename T>
	auto view() -> T& 
	{
		return *reinterpret_cast<T*>(this->blob());
	}

	template <typename T>
	auto view() const -> T const&
	{
		return *reinterpret_cast<T const*>(this->blob());
	}

	auto blob() -> void* 
	{
		return reinterpret_cast<void*>(buffer.data());
	}

	auto blob() const -> void const* 
	{
		return reinterpret_cast<void const*>(buffer.data());
	}
};

struct FrameBasedValue : ValueBase
{
	size_t stackOffset;

	template <typename T>
	auto view(StackFrame const& frame_) -> T&
	{
		return *reinterpret_cast<T*>(this->blob(frame_));
	}

	template <typename T>
	auto view(StackFrame const& frame_) const -> T const&
	{
		return *reinterpret_cast<T*>(this->blob(frame_));
	}

	// Temp: ??
	auto blob(StackFrame const& frame_) const -> void const* 
	{
		return static_cast<void const*>(frame_.stack->data() + frame_.initialStackSize + stackOffset);
	}

	auto toAbsolute(StackFrame const& frame_) const -> Value
	{
		return Value{ type, const_cast<void*>(this->blob(frame_)) };
	}
};

using OptValue = std::optional<Value>;

using ConversionFunc = OptValue(Instance &, Value const&);

template <typename T>
auto addTypeConversion(Instance &vm_, Scope& universeScope_, DeclType const& from_, DeclType const& to_, ConversionFunc& func_) -> void;
template <typename T>
auto addTypeConversion(Instance &vm_, Scope& universeScope_, std::string_view from_, std::string_view to_, ConversionFunc& func_) -> void;
}
