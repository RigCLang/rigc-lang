#pragma once

#include <RigCParser/RigCParserPCH.hpp>

#include <RigCParser/Grammar/Parts.hpp>

namespace rigc
{

struct Statement;
struct SingleBlockStatement;
struct CodeBlock;
struct IfStatement;

struct Condition
	:
	p::seq< p::one<'('>, OptWs, Expression, OptWs, p::one<')'> >
{};

struct OnlyIfStatement
	:
	p::seq< IfKeyword, OptWs, Condition, OptWs,
		p::sor<
			SingleBlockStatement,
			CodeBlock
		>
	>
{
};

struct ElseStatement
	:
	p::seq< ElseKeyword,
		p::sor<
			p::seq<Ws, IfStatement>,
			p::seq<OptWs,
				p::sor<
					SingleBlockStatement,
					CodeBlock
				>
			>
		>
	>
{
};

struct IfStatement
	:
	p::seq< OnlyIfStatement, OptWs, p::opt<ElseStatement> >
{
};

}
