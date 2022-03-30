#pragma once

#include RIGCPARSER_PCH

#include <RigCParser/Grammar/Keywords.hpp>
#include <RigCParser/Grammar/Literals.hpp>

namespace rigc
{

struct PackageImportNames
	: p::seq< struct Name, p::opt< p::seq< p::one<'.'>, PackageImportNames> > >
{
};

struct PackageImportFullName
	: p::must< PackageImportNames >
{
};

struct Name
	: p::identifier
{};

struct Type;

struct TemplateParam
	:
	p::seq<Type>
{
};

struct TemplateParamsInner
	:
	p::seq<
		TemplateParam,
		p::opt< opt_ws,
			p::opt<  p::seq< p::one<','>, opt_ws, TemplateParam>  >
		>
	>
{
};

struct TemplateParams
	:
	p::if_must< p::one<'<'>, opt_ws, TemplateParamsInner, opt_ws, p::one<'>'> >
{
};

struct Type
	:
	p::sor< IntegerLiteral, p::seq< Name, p::opt<opt_ws, TemplateParams> > >
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
