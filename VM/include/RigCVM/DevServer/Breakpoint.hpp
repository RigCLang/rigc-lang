#pragma once

#include RIGCVM_PCH

namespace rigc::vm
{

struct Breakpoint {
	int id;
	size_t line;
	size_t column;
	bool verified;
};


}
