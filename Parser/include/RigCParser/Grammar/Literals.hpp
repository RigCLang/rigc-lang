#pragma once

#include RIGCPARSER_PCH

#include <RigCParser/Grammar/Keywords.hpp>
#include <RigCParser/Grammar/Tokens.hpp>

namespace rigc
{


struct Digits
	: p::plus<p::digit>
{
};

struct IntegerLiteral
	: Digits
{
};

struct Float64Literal
	: p::seq<IntegerLiteral, p::one<'.'>, p::star<Digits> >
{
};

struct Float32Literal
	: p::seq<Float64Literal, p::one<'f'> >
{
};

struct ListOfArrayElements;

struct BoolLiteral
	: p::sor<TrueKeyword, FalseKeyword>
{
};

struct ArrayLiteral
	: p::seq< p::one<'['>, opt_ws, ListOfArrayElements, opt_ws, p::one<']'> >
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
	p::if_must< p::one<'"'>, p::star<StringLiteralContents>, p::one<'"'> >
{
};


}