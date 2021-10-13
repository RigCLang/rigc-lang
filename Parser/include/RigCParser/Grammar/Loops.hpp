#pragma once

#include RIGCPARSER_PCH

#include <RigCParser/Grammar/Parts.hpp>

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