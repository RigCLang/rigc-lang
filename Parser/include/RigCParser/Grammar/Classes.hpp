#pragma once

#include RIGCPARSER_PCH

#include <RigCParser/Grammar/Functions.hpp>
#include <RigCParser/Grammar/Templates.hpp>

namespace rigc
{

struct ExplicitType
	: p::seq< p::one<':'>, OptWs, Type >
{};

struct DataMemberDef
	: p::seq< Name, OptWs, p::sor<p::seq<ExplicitType, p::opt<Initialization>>, Initialization>, OptWs, p::one<';'> >
{};

struct MethodDef
	: p::seq< p::opt< OverrideKeyword, Ws >, p::opt<TemplateDefPreamble, OptWs>, Name, OptWs, p::opt<FunctionParams>, p::opt<OptWs, ExplicitReturnType>, OptWs, CodeBlock >
{};

struct MemberDef
	: p::sor<
		DataMemberDef,
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


}
