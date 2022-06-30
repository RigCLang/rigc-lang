#pragma once

#include RIGCVM_PCH

#include <RigCVM/Value.hpp>


namespace rigc::vm
{
struct Stack;
struct Scope;

struct StackFrame
{
	Stack const*	stack;
	Scope*			scope;
	std::size_t		initialStackSize = 0;

	Vec< Value >	allocatedValues;
};

struct StackFramePusher
{
	StackFramePusher(Instance& vm_, ParserNode const& stmt_);
	~StackFramePusher();

	Instance& vm;
};

}
