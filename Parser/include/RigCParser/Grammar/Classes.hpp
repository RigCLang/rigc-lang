#pragma once

#include RIGCPARSER_PCH

#include <RigCParser/Grammar/Functions.hpp>

namespace rigc
{

struct ExplicitType
	: p::seq<p::one<':'>, opt_ws, Type>
{};

struct DataMemberDef
	: p::seq< Name, opt_ws, p::sor<p::seq<ExplicitType, p::opt<Initialization>>, Initialization>, opt_ws, p::one<';'> >
{};

struct MethodDef
	: p::seq< p::opt< OverrideKeyword, ws >, Name, opt_ws, p::opt<FunctionParams>, opt_ws, CodeBlock >
{};

struct MemberDef
	: p::sor<
		DataMemberDef,
		MethodDef
	>
{
};

struct ClassCodeBlock
	: p::seq< p::one<'{'>, opt_ws, p::star<MemberDef, opt_ws>, p::one<'}'> >
{
};

struct ClassDefinition
	: p::seq< p::opt<ExportKeyword, ws>, p::if_must< ClassKeyword, ws, Name, opt_ws, ClassCodeBlock > >
{
};


}
