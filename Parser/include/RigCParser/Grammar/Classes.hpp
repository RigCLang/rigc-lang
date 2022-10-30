#pragma once

#include <RigCParser/RigCParserPCH.hpp>

#include <RigCParser/Grammar/Functions.hpp>
#include <RigCParser/Grammar/Templates.hpp>

namespace rigc
{

struct ExplicitType
	: p::seq< p::one<':'>, OptWs, Type >
{};

struct DataMemberDef
	: p::seq< Name, OptWs, p::sor<p::seq<ExplicitType, p::opt<Initialization>>, p::opt<Initialization>>, OptWs>
{};

struct MethodDef
	: p::seq< p::opt< OverrideKeyword, Ws >, p::opt<TemplateDefPreamble, OptWs>, Name, OptWs, p::opt<FunctionParams>, p::opt<OptWs, ExplicitReturnType>, OptWs, CodeBlock >
{};

struct MemberDef
	: p::sor<
		p::seq<DataMemberDef, p::one<';'>>,
		MethodDef
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
			p::opt<TemplateDefPreamble, OptWs>,
			p::if_must< ClassKeyword, Ws, Name, OptWs, ClassCodeBlock> >
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
