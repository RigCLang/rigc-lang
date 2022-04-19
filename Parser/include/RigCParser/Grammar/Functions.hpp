#pragma once

#include RIGCPARSER_PCH

#include <RigCParser/Grammar/Parts.hpp>

namespace rigc
{

struct Statements;

struct CodeBlock
	: p::seq< p::one<'{'>, opt_ws, Statements, opt_ws, p::one<'}'> >
{
};

struct ReturnStatement
	:
	p::if_must< RetKeyword, opt_ws, Expression >
{};

struct ExplicitReturnTypeArrow
	: p::string<'-','>'>
{};

struct ExplicitReturnType
	:
	p::if_must< ExplicitReturnTypeArrow, opt_ws, Type >
{};

struct Parameter
	: p::seq<
		Name,
		p::opt< opt_ws, p::one<':'>, opt_ws, Type >,
		p::opt< Assignment >
	>
{
};

struct Params
	:
	p::seq< Parameter, opt_ws, p::star< p::one<','>, opt_ws, Parameter > >
{
};

struct FunctionParams
	: p::seq< p::one<'('>, opt_ws, p::opt<Params>, opt_ws, p::one<')'> >
{
};

struct FunctionDefinition
	: p::seq<
			p::opt<ExportKeyword, ws>,
			p::if_must<
					FuncKeyword, ws, Name, opt_ws, p::opt<FunctionParams>, p::opt<opt_ws, ExplicitReturnType>, opt_ws, CodeBlock
				>
		>
{
};

struct ClosureDefinition
	: p::seq<
		p::sor<
			Name,
			FunctionParams
		>,
		opt_ws,
		p::if_must<
			p::string<'=','>'>,
			opt_ws,
			p::sor<
				Expression,
				CodeBlock
			>
		>
	>
{};


}
