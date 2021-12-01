#pragma once

namespace rigc::vm
{

struct Stack;
struct Scope;

struct StackFrame
{
	Stack const*	stack;
	Scope*			scope;
	size_t			initialStackSize = 0;
};


}
