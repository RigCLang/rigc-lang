#pragma once

#include RIGC_PCH

#include <RigC/Grammar/Parts.hpp>

namespace rigc
{

struct Statements;

struct CodeBlock
	: p::seq< p::one<'{'>, opt_ws, Statements, opt_ws, p::one<'}'> >
{
};

struct ParamDefinition
	: p::seq<
		Name,
		p::opt< opt_ws, p::one<':'>, opt_ws, Type >,
		p::opt< Assignment >
	>
{
};

struct Params
	:
	p::seq< ParamDefinition, opt_ws, p::opt< p::one<','>, opt_ws, ParamDefinition > >
{
};

struct FunctionParams
	: p::seq< p::one<'('>, opt_ws, p::opt<Params>, opt_ws, p::one<')'> >
{
};

struct FunctionDefinition
	: p::seq< FuncKeyword, ws, Name, opt_ws, p::opt<FunctionParams>, opt_ws, CodeBlock >
{
};


}