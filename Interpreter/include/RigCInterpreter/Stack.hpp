#pragma once

#include <RigCInterpreter/StackFrame.hpp>

namespace rigc::vm
{

struct Stack
{
	using DataContainer		= std::vector<char>;
	using FrameContainer	= std::vector<StackFrame>;

	DataContainer	container;
	FrameContainer	frames;

	size_t		size = 0;

	char* data() {
		return container.data();
	}
	char const* data() const {
		return container.data();
	}

	StackFrame& pushFrame()
	{
		StackFrame frame{this};
		frame.initialStackSize = this->size;
		frames.push_back(frame);
		return frames.back();
	}

	void popFrame()
	{
		size = frames.back().initialStackSize;
		frames.pop_back();
	}
};

}
