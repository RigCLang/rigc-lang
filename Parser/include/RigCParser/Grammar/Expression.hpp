#pragma once

#include RIGCPARSER_PCH

#include <RigCParser/Grammar/Keywords.hpp>
#include <RigCParser/Grammar/Tokens.hpp>

namespace rigc
{

struct Expression;
struct ExpressionCore;
struct ExprInParen;

struct TwoSidedOp
	: p::seq<opt_ws, TwoArgOper, opt_ws, Expression>
{
};

struct SelectOp
	: p::seq<opt_ws, p::one<'?'>, opt_ws, Expression, opt_ws, p::one<':'>, Expression>
{
};

struct Digits
	: p::plus<p::digit>
{
};

struct IntegerLiteral
	: Digits
{
};

struct EscapeSequence
	: p::seq< p::one<'\\'>, p::any >
{
};

struct StringLiteralContents
	:
	p::sor<
		EscapeSequence,
		p::not_one<'\n', '\"'>
	>
{
};

struct StringLiteral
	:
	p::seq< p::one<'"'>, p::star<StringLiteralContents>, p::one<'"'> >
{
};

struct VariableModification2S
	: p::seq< Name, opt_ws, ModOper2S, opt_ws, Expression>
{
};

struct VariableModification1S
	:
	p::sor<
		p::seq< ModOper1S, opt_ws, ExprInParen >,
		p::seq< ExprInParen , opt_ws, ModOper1S>
	>
{
};

struct VariableModification
	: p::sor<
		VariableModification1S,
		VariableModification2S
	>
{
};


struct ExpressionCore
	:
	p::seq<
		p::sor<
			IntegerLiteral,
			StringLiteral,
			VariableModification,
			struct FunctionCall,
			p::identifier
		>,
		p::opt<
			p::sor<
				SelectOp,
				TwoSidedOp
			>
		>
	>
{
};

struct ExprInParen
	: p::seq< p::one<'('>, opt_ws, ExpressionCore, opt_ws, p::one<')'> >
{
};

struct Expression
	: p::sor< ExprInParen, ExpressionCore >
{
};

struct Arguments
	:
	p::seq< Expression, opt_ws, p::opt< p::one<','>, opt_ws, Arguments > >
{
};

struct FunctionCall
	:
	p::seq<Name, opt_ws, p::one<'('>, opt_ws, p::opt< Arguments >, opt_ws, p::one<')'> >
{
};

}