#pragma once

#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Type.hpp>
#include <RigCInterpreter/Stack.hpp>

namespace rigc::vm
{

template <typename T>
struct Ref
{
	Ref() = default;

	Ref(T* ptr_)
		: ptr(ptr_)
	{}

	T* ptr;

	T&			operator*()			{ return *ptr; }
	T const&	operator*()	const	{ return *ptr; }

	bool		operator!() const	{ return ptr == nullptr; }

	explicit operator bool() const {
		return static_cast<bool>(ptr);
	}

	operator T*() const {
		return ptr;
	}

	operator T const*() const {
		return ptr;
	}
};

template <typename T>
using Ptr = T*;

struct ValueBase
{
	DeclType	type;

	DeclType const& getType() const {
		return type;
	}

	std::string_view typeName() const {
		return type.decay().type->name;
	}

	std::string fullTypeName() const
	{
		return type.name();
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


}
