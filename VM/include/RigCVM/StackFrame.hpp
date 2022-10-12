#pragma once

#include RIGCVM_PCH

#include <RigCVM/Value.hpp>


namespace rigc::vm
{
struct Stack;
struct Scope;

/// @brief Represents a single stack frame.
struct StackFrame
{
	Stack const*		stack;
	Scope*				scope;

	/// @brief The size of the stack when the frame was created.
	size_t initialStackSize = 0;

	/// @brief Values allocated during the execution of this frame.
	/// @note VM is responsible for ending their lifetime.
	/// @note In Release mode only class types are stored here (TODO: rethink)
	/// 	because primitive values don't have destructors.
	DynArray<Value> allocatedValues;
};

struct StackFramePusher
{
	StackFramePusher(Instance& vm_, ParserNode const& stmt_);
	~StackFramePusher();

	Instance& vm;
};

}
