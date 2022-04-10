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
		p::star<PrefixOperator>,opt_ws,
		p::sor<AnyLiteral, Name, ExprInParen>,opt_ws,
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
			opt_ws, p::if_must<TInfixOperator, opt_ws, SingleExpressionFragment>
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
	: p::if_must<p::one<'('>, opt_ws, Expression, opt_ws, p::one<')'> >
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

}
