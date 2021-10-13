#pragma once

#include RIGCPARSER_PCH

#include <RigCParser/Grammar/Parts.hpp>
#include <RigCParser/Grammar/Variables.hpp>
#include <RigCParser/Grammar/Conditionals.hpp>
#include <RigCParser/Grammar/Loops.hpp>
#include <RigCParser/Grammar/Functions.hpp>

namespace rigc
{

struct NonExpression
	:
	p::sor<
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
		WhileStatement
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


struct Statements
	: p::star< p::seq<opt_ws, Statement > >
{
};

struct ImportStatement
	: p::seq< FromKeyword, ws, PackageImportFullName, ws, ImportKeyword, ws, p::identifier, Semicolon>
{
};

}