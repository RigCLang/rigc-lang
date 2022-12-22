#pragma once

#include <RigCParser/RigCParserPCH.hpp>

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
	p::star< p::sor<ImportStatement, ClassDefinition, FunctionDefinition, UnionDefinition, EnumDefinition, Ws> >
{
};

struct Grammar
	: p::must<GlobalNS, OptWs, p::eof>
{
};

template< typename Rule >
using Selector = p::parse_tree::selector< Rule,
	p::parse_tree::store_content::on<
			ImportStatement,
			PackageImportFullName,
			ClassDefinition,
			ClassCodeBlock,
			UnionDefinition,
			UnionCodeBlock,
			EnumDefinition,
			EnumCodeBlock,
			MethodDef,
			MemberOperatorDef,
			DataMemberDef,
			ExplicitType,
			FunctionDefinition,
			ExplicitReturnType,
			VariableDefinition,
			InitializerValue,
			IfStatement,
			ElseStatement,
			WhileStatement,
			ForStatement,
			ReturnStatement,
			BreakStatement,
			ContinueStatement,
			CodeBlock,
			Statements,
			SingleBlockStatement,
			Condition,
			Expression,
			ArrayElement,
			DeclType,
			FunctionParams,
			ClosureDefinition,
			Parameter,
			FunctionArg,
			Type,
			PossiblyTemplatedSymbol,
			PossiblyTemplatedSymbolNoDisamb,
			TemplateDefParamList,
			TemplateDefParamListElem,
			TemplateDefParamKind,
			TemplateParams,
			TemplateParam,
			Name,
			OverridableOperatorNames,
			IntegerLiteral,
			Float32Literal,
			Float64Literal,
			PrefixOperator,
			InfixOperator,
			InfixOperatorNoComma,
			PostfixOperator,
			BoolLiteral,
			StringLiteral,
			CharLiteral,
			ArrayLiteral,
			ExportKeyword,
			ListOfFunctionArguments
		>
	>;

}
