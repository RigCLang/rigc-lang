#pragma once

#include RIGC_PCH

#include <RigC/Grammar/Parts.hpp>
#include <RigC/Grammar/Variables.hpp>
#include <RigC/Grammar/Conditionals.hpp>
#include <RigC/Grammar/Loops.hpp>
#include <RigC/Grammar/Functions.hpp>

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