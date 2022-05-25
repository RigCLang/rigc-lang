#pragma once

#include RIGCPARSER_PCH

#include <RigCParser/Grammar/Functions.hpp>
#include <RigCParser/Grammar/Templates.hpp>
#include <RigCParser/Grammar/Keywords.hpp>
#include <RigCParser/Grammar/Operators.hpp>

namespace rigc
{

struct ExplicitType
	: p::seq< p::one<':'>, OptWs, Type >
{};

struct DataMemberDef
	: p::seq< Name, OptWs, p::sor<p::seq<ExplicitType, p::opt<Initialization>>, p::opt<Initialization>>, OptWs>
{};

struct MethodDef
	: p::seq< p::opt< OverrideKeyword, Ws >, Name, OptWs, p::opt<FunctionParams>, p::opt<OptWs, ExplicitReturnType>, OptWs, CodeBlock >
{};

struct OverloadedEntity
	: p::sor<
			p::seq<AsKeyword, Ws, Name>
		>
{
};

struct OperatorDef
	: p::if_must<OperatorKeyword, Ws, OverloadedEntity, Ws, p::opt<FunctionParams>, OptWs, CodeBlock>
{
};

struct MemberDef
	: p::sor<
		p::seq<DataMemberDef, p::one<';'>>,
		MethodDef,
		OperatorDef
	>
{
};

struct ClassCodeBlock
	: p::seq< p::one<'{'>, OptWs, p::star<MemberDef, OptWs>, p::one<'}'> >
{
};

struct ClassDefinition
	: p::seq<
			p::opt<ExportKeyword, Ws>, 
			p::opt<TemplateDefPreamble, Ws>,
			p::if_must< ClassKeyword, Ws, Name, OptWs, ClassCodeBlock > >
{
};


struct EnumCodeBlock
	: p::seq< p::one<'{'>, OptWs, p::opt<p::star<p::seq<DataMemberDef, p::one<';'>>, OptWs>>, p::one<'}'> >
{
};

using EnumExplicitType = p::if_must<Ws, OfKeyword, Ws, Name>;

struct EnumDefinition
	: p::seq<
			p::opt<ExportKeyword, Ws>,
			p::if_must< EnumKeyword, Ws, Name, p::opt<EnumExplicitType>, OptWs, EnumCodeBlock > >
{
};

struct UnionCodeBlock
	: p::seq< p::one<'{'>, OptWs, p::star<p::seq<DataMemberDef, p::one<';'>>, OptWs>, p::one<'}'> >
{
};

struct UnionDefinition
	: p::seq<
			p::opt<ExportKeyword, Ws>, 
			p::opt<TemplateDefPreamble, Ws>,
			p::if_must< UnionKeyword, Ws, Name, OptWs, UnionCodeBlock > >
{
};

}
