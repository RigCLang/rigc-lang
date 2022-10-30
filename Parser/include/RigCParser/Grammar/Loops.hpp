#pragma once
#include <RigCParser/RigCParserPCH.hpp>

#include <RigCParser/Grammar/Parts.hpp>
#include <RigCParser/Grammar/Variables.hpp>
#include <RigCParser/Grammar/Characters.hpp>

namespace rigc
{

struct Statement;
struct SingleBlockStatement;
struct CodeBlock;
struct Condition;

struct WhileStatement
	:
	p::seq< WhileKeyword, OptWs, Condition, OptWs,
		p::sor<
			SingleBlockStatement,
			CodeBlock
		>
	>
{
};


struct ForStatement
	:
	p::seq<
		ForKeyword,
		WsWrapped<
			p::one<'('>,
				WsWrapped<VariableDefinition>, p::one<';'>,
				WsWrapped<Expression>, p::one<';'>,
				WsWrapped<Expression>,
			p::one<')'>
		>,
		p::sor<
			SingleBlockStatement,
			CodeBlock
		>
	>
{
};

}
