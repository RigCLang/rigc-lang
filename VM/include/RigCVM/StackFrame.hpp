#pragma once

#include <cstdint>

namespace rigc::vm
{

struct Stack;
struct Scope;

struct StackFrame
{
	Stack const*	stack;
	Scope*			scope;
	std::size_t			initialStackSize = 0;
};


}
