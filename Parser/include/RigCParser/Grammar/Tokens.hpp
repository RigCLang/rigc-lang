#pragma once

#include RIGCPARSER_PCH

#include <RigCParser/Grammar/Keywords.hpp>
#include <RigCParser/Grammar/Literals.hpp>
#include <RigCParser/Grammar/Characters.hpp>

namespace rigc
{

struct PackageImportNames
	: p::seq< struct Name, p::opt< p::seq< p::one<'.'>, PackageImportNames> > >
{
};

struct PackageImportFullName
	:
	p::sor<
		StringLiteral,
		PackageImportNames
	>
{
};

struct Name
	: p::identifier
{};

struct Type;

struct TemplateParam
	:
	p::sor<Type, IntegerLiteral, Float64Literal, Float32Literal>
{
};

struct TemplateParamsInner
	:
	p::seq<
		TemplateParam,
		p::opt< OptWs,
			p::opt<  p::seq< p::one<','>, OptWs, TemplateParam>  >
		>
	>
{
};

struct TemplateParams
	:
	p::if_must< p::one<'<'>, OptWs, TemplateParamsInner, OptWs, p::one<'>'> >
{
};

struct Type
	:
	p::seq< Name, p::opt<OptWs, TemplateParams> >
{};

struct DeclType
	:
	p::sor<
		VarKeyword,
		ConstKeyword,
		Type
	>
{};

}
