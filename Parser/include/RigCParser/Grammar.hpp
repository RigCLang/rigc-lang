#pragma once

#include RIGCPARSER_PCH

#include <RigCParser/Grammar/Characters.hpp>
#include <RigCParser/Grammar/Keywords.hpp>
#include <RigCParser/Grammar/Statements.hpp>
#include <RigCParser/Grammar/Tokens.hpp>

namespace rigc
{

// Parsing rule that matches a non-empty sequence of
// alphabetic ascii-characters with greedy-matching.

struct GlobalNS
	:
	p::star< p::sor<ImportStatement, FunctionDefinition, ws> >
{
};

struct Grammar
	: p::must<GlobalNS, opt_ws, p::eof>
{
};

template< typename Rule >
using Selector = p::parse_tree::selector< Rule,
	p::parse_tree::store_content::on<
			ImportStatement,
			FunctionDefinition,
			VariableDefinition,
			InitializerValue,
			FunctionCall,
			IfStatement,
			WhileStatement,
			ReturnStatement,
			CodeBlock,
			Statements,
			Condition,
			Expression,
			DeclType,
			FunctionParams,
			Parameter,
			FunctionArg,
			ArrayElement,
			Type,
			Name,
			PrefixOperator,
			InfixOperator,
			InfixOperatorNoComma,
			PostfixOperator,
			IntegerLiteral,
			StringLiteral
		>
	>;

}