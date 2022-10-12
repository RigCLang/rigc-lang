#pragma once

#include RIGCVM_PCH

#include <RigCVM/StackFrame.hpp>
#include <RigCVM/DevServer/Messaging.hpp>

namespace rigc::vm
{

struct Stack
{
	using DataContainer		= DynArray<char>;
	using FrameContainer	= DynArray<StackFrame>;

	DataContainer	container;
	FrameContainer	frames;

	// Do not use stack.size(), resize etc to
	// be 100% sure that it won't reallocate
	size_t		size = 0;

	auto data() -> char*
	{
		return container.data();
	}

	auto data() const -> char const*
	{
		return container.data();
	}

	auto pushFrame() -> StackFrame&
	{
		auto frame = StackFrame(this);
		frame.initialStackSize = this->size;
		frames.push_back(frame);

		return frames.back();
	}

	auto popFrame() -> void
	{
		size = frames.back().initialStackSize;
		frames.pop_back();
	}
};

}
