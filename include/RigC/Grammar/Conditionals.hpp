#pragma once

#include RIGC_PCH

#include <RigC/Grammar/Parts.hpp>

namespace rigc
{

struct Statement;
struct CodeBlock;
struct IfStatement;

struct Condition
	:
	p::seq< p::one<'('>, opt_ws, Expression, opt_ws, p::one<')'> >
{};

struct OnlyIfStatement
	:
	p::seq< IfKeyword, opt_ws, Condition, opt_ws,
		p::sor<
			Statement,
			CodeBlock
		>
	>
{
};

struct ElseStatement
	:
	p::seq< ElseKeyword,
		p::sor<
			p::seq<ws, IfStatement>,
			p::seq<opt_ws,
				p::sor<
					Statement,
					CodeBlock
				>
			>
		>
	>
{
};

struct IfStatement
	:
	p::seq< OnlyIfStatement, opt_ws, p::opt<ElseStatement> >
{
};

}