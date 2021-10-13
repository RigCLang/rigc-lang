#pragma once

#include RIGC_PCH

#include <RigC/Grammar/Keywords.hpp>

namespace rigc
{

struct PackageImportNames
	: p::seq< p::identifier, p::opt< p::seq< p::one<'.'>, PackageImportNames> > >
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
	p::seq< p::one<'<'>, opt_ws, TemplateParamsInner, opt_ws, p::one<'>'> >
{
};

struct Type
	:
	p::seq< Name, p::opt<opt_ws, TemplateParams> >
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