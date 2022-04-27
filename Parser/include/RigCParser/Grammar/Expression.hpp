#pragma once

#include RIGCPARSER_PCH

#include <RigCParser/Grammar/Keywords.hpp>
#include <RigCParser/Grammar/Tokens.hpp>
#include <RigCParser/Grammar/Literals.hpp>
#include <RigCParser/Grammar/Operators.hpp>

namespace rigc
{

struct Expression;
struct ExprInParen;

struct SingleExpressionFragment
	:
	p::seq<
		p::star<PrefixOperator>,OptWs,
		p::sor<AnyLiteral, Name, ExprInParen>,OptWs,
		p::star<PostfixOperator>
	>
{
};

template <typename TInfixOperator = InfixOperator>
struct ExpressionBase
	:
	p::seq<
		SingleExpressionFragment,
		p::star<
			OptWs, p::if_must<TInfixOperator, OptWs, SingleExpressionFragment>
		>
	>
{
};

struct Expression
	: ExpressionBase<InfixOperator>
{};

struct ExprWithoutComma
	: ExpressionBase<InfixOperatorNoComma>
{};

struct ExprInParen
	: p::if_must<p::one<'('>, OptWs, Expression, OptWs, p::one<')'> >
{
};

struct ArrayElement	: ExprWithoutComma {};
struct FunctionArg	: ExprWithoutComma {};

template <typename ElementType>
struct ListOfExpressions
	: p::opt<
		ElementType, OptWs,
		p::star<
			p::seq<
				CommaOp, OptWs,
				ElementType
			>
		>,
		OptWs,
		p::opt<CommaOp>
	>
{
};

struct ListOfArrayElements		: ListOfExpressions<ArrayElement> {};
struct ListOfFunctionArguments	: ListOfExpressions<FunctionArg> {};

}
