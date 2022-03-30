#pragma once

#include RIGCPARSER_PCH

#include <RigCParser/Grammar/Characters.hpp>
#include <RigCParser/Grammar/Keywords.hpp>
#include <RigCParser/Grammar/Statements.hpp>
#include <RigCParser/Grammar/Tokens.hpp>
#include <RigCParser/Grammar/Classes.hpp>

namespace rigc
{

// Parsing rule that matches a non-empty sequence of
// alphabetic ascii-characters with greedy-matching.

struct GlobalNS
	:
	p::star< p::sor<ImportStatement, ClassDefinition, FunctionDefinition, ws> >
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
			ClassDefinition,
			ClassCodeBlock,
			MethodDef,
			FunctionDefinition,
			VariableDefinition,
			InitializerValue,
			FunctionCall,
			IfStatement,
			ElseStatement,
			WhileStatement,
			ReturnStatement,
			CodeBlock,
			Statements,
			SingleBlockStatement,
			Condition,
			Expression,
			DeclType,
			FunctionParams,
			ClosureDefinition,
			Parameter,
			FunctionArg,
			ArrayElement,
			Type,
			Name,
			IntegerLiteral,
			Float32Literal,
			Float64Literal,
			PrefixOperator,
			InfixOperator,
			InfixOperatorNoComma,
			PostfixOperator,
			BoolLiteral,
			StringLiteral,
			ArrayLiteral,
			ListOfFunctionArguments
		>
	>;

}
