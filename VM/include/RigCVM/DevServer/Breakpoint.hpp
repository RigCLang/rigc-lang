#pragma once

#include <RigCVM/RigCVMPCH.hpp>

namespace rigc::vm
{

struct Breakpoint {
	int id;
	size_t line;
	size_t column;
	bool verified;
};


}
