#pragma once

#include RIGCPARSER_PCH

#include <RigCParser/Grammar/Keywords.hpp>
#include <RigCParser/Grammar/Tokens.hpp>
#include <RigCParser/Grammar/Operators.hpp>

namespace rigc
{

struct Expression;
struct ExprInParen;

struct Digits
	: p::plus<p::digit>
{
};

struct IntegerLiteral
	: Digits
{
};

struct ListOfArrayElements;

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

template <typename... Operators>
struct AtomicExprPart
	:
	p::seq<
		opt_ws,
		p::sor<
			Operators...,
			ArrayLiteral,
			IntegerLiteral,
			StringLiteral,
			struct ClosureDefinition,
			struct FunctionCall,
			Name
		>,
		opt_ws
	>
{
};

struct AtomicExprPartFirst		: AtomicExprPart<> {};
struct AtomicExprPartMid		: AtomicExprPart<InfixOperator> {};
struct AtomicExprPartMidNoComma	: AtomicExprPart<InfixOperatorNoComma> {};

template <typename AtomicsMid>
struct ExpressionBase
	:
	p::seq<
		p::sor<
				p::plus<PrefixOperator>,
				AtomicExprPartFirst,
				ExprInParen
			>,
		p::star<
			p::sor<
				AtomicsMid,
				ExprInParen
			>
		>,
		p::star<
			PostfixOperator
		>
	>
{
};

struct Expression
	: ExpressionBase<AtomicExprPartMid>
{};

struct ExprWithoutComma
	: ExpressionBase<AtomicExprPartMidNoComma>
{};

struct ExprInParen
	: p::seq<p::one<'('>, opt_ws, Expression, opt_ws, p::one<')'> >
{
};

struct ArrayElement	: ExprWithoutComma {};
struct FunctionArg	: ExprWithoutComma {};

template <typename ElementType>
struct ListOfExpressions
	: p::opt<
		ElementType, opt_ws,
		p::star<
			p::seq<
				CommaOp, opt_ws,
				ElementType
			>
		>,
		opt_ws,
		p::opt<CommaOp>
	>
{
};

struct ListOfArrayElements		: ListOfExpressions<ArrayElement> {};
struct ListOfFunctionArguments	: ListOfExpressions<FunctionArg> {};

struct FunctionCall
	:
	p::seq<
		p::sor<Name, ExprInParen>,
		opt_ws, p::one<'('>, opt_ws, p::opt< ListOfFunctionArguments >, opt_ws, p::one<')'>
	>
{
};

}