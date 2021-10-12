#pragma once

#include RIGC_PCH

#include <RigC/Grammar/Parts.hpp>

namespace rigc
{

struct Statement;
struct CodeBlock;
struct Condition;

struct WhileStatement
	:
	p::seq< WhileKeyword, opt_ws, Condition, opt_ws,
		p::sor<
			Statement,
			CodeBlock
		>
	>
{
};


}