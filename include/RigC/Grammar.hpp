#pragma once

#include RIGC_PCH

#include <RigC/Grammar/Characters.hpp>
#include <RigC/Grammar/Keywords.hpp>
#include <RigC/Grammar/Statements.hpp>
#include <RigC/Grammar/Tokens.hpp>

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
			VariableModification,
			FunctionCall,
			IfStatement,
			WhileStatement,
			DeclType,
			Type,
			Name
		>
	>;

}