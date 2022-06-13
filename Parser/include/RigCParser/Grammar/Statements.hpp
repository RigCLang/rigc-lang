#pragma once

#include RIGCPARSER_PCH

#include <RigCParser/Grammar/Parts.hpp>
#include <RigCParser/Grammar/Variables.hpp>
#include <RigCParser/Grammar/Conditionals.hpp>
#include <RigCParser/Grammar/Loops.hpp>
#include <RigCParser/Grammar/Functions.hpp>

namespace rigc
{

struct BreakStatement
	: p::if_must< BreakKeyword, p::sor< p::seq< Ws, IntegerLiteral >, OptWs > >
{
};

struct ContinueStatement
	: ContinueKeyword
{
};

struct NonExpression
	:
	p::sor<
		ReturnStatement,
		BreakStatement,
		ContinueStatement,
		VariableDefinition
	>
{
};

struct SingleStatement
	:
	p::seq< p::sor<NonExpression, Expression>, Semicolon >
{
};

struct ComplexStatement
	:
	p::sor<
		IfStatement,
		WhileStatement,
		ForStatement
	>
{
};

struct Statement
	:
	p::sor<
		ComplexStatement,
		SingleStatement
	>
{};


struct SingleBlockStatement
	: Statement
{
};

struct Statements
	: p::star< p::seq<OptWs, Statement > >
{
};

struct ImportStatement
	: p::if_must< ImportKeyword, Ws, PackageImportFullName, OptWs, Semicolon>
{
};

}
