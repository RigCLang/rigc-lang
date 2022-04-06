#pragma once

#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Type.hpp>
#include <RigCInterpreter/Stack.hpp>

namespace rigc::vm
{

struct Instance;
struct DataMember;

template <typename T>
using Ptr = T*;

struct ValueBase
{
	DeclType	type;

	DeclType const& getType() const {
		return type;
	}

	std::string typeName() const {
		return type->decay()->name();
	}

	std::string fullTypeName() const
	{
		return type->name();
	}
};

struct Value
	: ValueBase
{
	void* data;

	template <typename T>
	T& view() {
		return *reinterpret_cast<T*>(data);
	}

	template <typename T>
	T const& view() const {
		return *reinterpret_cast<T*>(data);
	}

	// Temp:
	void* blob() const {
		return data;
	}

	Value member(DataMember const& dm_) const;

	Value member(size_t offset_, DeclType type_) const;

	Value safeRemoveRef() const;
	Value safeRemovePtr() const;
	Value removeRef() const;
	Value removePtr() const;
};

std::string dump(Instance& vm_, Value const& value_);

struct CompileTimeValue
	: ValueBase
{
	std::vector<std::byte> buffer;

	template <typename T>
	T& view() {
		return *reinterpret_cast<T*>(this->blob());
	}

	template <typename T>
	T const& view() const {
		return *reinterpret_cast<T const*>(this->blob());
	}

	void* blob() {
		return reinterpret_cast<void*>(buffer.data());
	}
	void const* blob() const {
		return reinterpret_cast<void const*>(buffer.data());
	}
};

struct FrameBasedValue
	: ValueBase
{
	size_t stackOffset;

	template <typename T>
	T& view(StackFrame const& frame_) {
		return *reinterpret_cast<T*>(this->blob(frame_));
	}

	template <typename T>
	T const& view(StackFrame const& frame_) const {
		return *reinterpret_cast<T*>(this->blob(frame_));
	}

	// Temp:
	void const* blob(StackFrame const& frame_) const
	{
		return static_cast<void const*>(frame_.stack->data() + frame_.initialStackSize + stackOffset);
	}

	Value toAbsolute(StackFrame const& frame_) const
	{
		return Value{ type, const_cast<void*>(this->blob(frame_)) };
	}
};

using OptValue = std::optional<Value>;


using ConversionFunc = OptValue(Instance &, Value const&);

template <typename T>
void addTypeConversion(Instance &vm_, Scope& universeScope_, DeclType const& from_, DeclType const& to_, ConversionFunc& func_);
template <typename T>
void addTypeConversion(Instance &vm_, Scope& universeScope_, std::string_view from_, std::string_view to_, ConversionFunc& func_);

}
