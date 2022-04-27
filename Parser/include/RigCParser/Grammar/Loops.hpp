#pragma once
#include RIGCPARSER_PCH

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
	p::seq< WhileKeyword, opt_ws, Condition, opt_ws,
		p::sor<
			SingleBlockStatement,
			CodeBlock
		>
	>
{
};

template<typename... GrammarElems>
struct WsWrapped 
	:
	p::seq<
		opt_ws,
		GrammarElems...,
		opt_ws
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
