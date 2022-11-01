#pragma once

#include <RigCParser/RigCParserPCH.hpp>

#include <RigCParser/Grammar/Keywords.hpp>
#include <RigCParser/Grammar/Tokens.hpp>
#include <RigCParser/Grammar/Characters.hpp>

namespace rigc
{


struct Digits
	: p::plus<p::digit>
{
};

struct IntegerLiteral
	: p::seq<p::opt<p::one<'-'>>, Digits>
{
};

struct Float64Literal
	: p::seq<IntegerLiteral, p::one<'.'>, p::star<Digits>, p::not_at<p::alnum> >
{
};

struct Float32Literal
	: p::seq<IntegerLiteral, p::one<'.'>, p::star<Digits>, p::one<'f'>, p::not_at<p::alnum> >
{
};

struct ListOfArrayElements;

struct BoolLiteral
	: p::sor<TrueKeyword, FalseKeyword>
{
};

struct ArrayLiteral
	: p::seq< p::one<'['>, OptWs, ListOfArrayElements, OptWs, p::one<']'> >
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
		p::not_one<'\n', '"'>
	>
{
};

struct StringLiteral
	:
	p::if_must< p::one<'"'>, p::star<StringLiteralContents>, p::one<'"'> >
{
};

struct CharLiteralContents
	:
	p::sor<
		EscapeSequence,
		p::not_one<'\n', '\''>
	>
{
};

struct CharLiteral
	:
	p::if_must< p::one<'\''>, p::star<CharLiteralContents>, p::one<'\''> >
{
};


struct AnyLiteral
	:
	p::sor<
		Float32Literal,
		Float64Literal,
		IntegerLiteral,
		BoolLiteral,
		StringLiteral,
		CharLiteral,
		ArrayLiteral
	>
{
};

}
