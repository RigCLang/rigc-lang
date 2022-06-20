#pragma once

#include RIGCPARSER_PCH

#include <RigCParser/Grammar/Parts.hpp>
#include <RigCParser/Grammar/Templates.hpp>

namespace rigc
{

struct Statements;

struct CodeBlock
	: p::seq< p::one<'{'>, OptWs, Statements, OptWs, p::one<'}'> >
{
};

struct ReturnStatement
	:
	p::if_must< RetKeyword, OptWs, Expression >
{};

struct ExplicitReturnTypeArrow
	: p::string<'-','>'>
{};

struct ExplicitReturnType
	:
	p::if_must< ExplicitReturnTypeArrow, OptWs, Type >
{};

struct Parameter
	: p::seq<
		Name,
		p::opt< OptWs, p::one<':'>, OptWs, Type >,
		p::opt< Assignment >
	>
{
};

struct Params
	:
	p::seq< Parameter, OptWs, p::star< p::one<','>, OptWs, Parameter > >
{
};

struct FunctionParams
	: p::seq< p::one<'('>, OptWs, p::opt<Params>, OptWs, p::one<')'> >
{
};


struct FunctionDefinition
	: p::seq<
			p::opt<ExportKeyword, Ws>,
			p::opt<TemplateDefPreamble, OptWs>,
			p::if_must<
					FuncKeyword, Ws, Name, OptWs, p::opt<FunctionParams>, p::opt<OptWs, ExplicitReturnType>, OptWs, CodeBlock
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
			OptWs,
			p::if_must<
				p::string<'=','>'>,
				OptWs,
				p::sor<
					Expression,
					CodeBlock
				>
			>
		>
{};


}
