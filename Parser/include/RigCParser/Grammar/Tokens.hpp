#pragma once

#include <RigCParser/RigCParserPCH.hpp>

#include <RigCParser/Grammar/Keywords.hpp>
#include <RigCParser/Grammar/Literals.hpp>
#include <RigCParser/Grammar/Characters.hpp>

namespace rigc
{

struct Name;

struct PackageImportNames
	: p::seq<
			Name,
			p::opt<
				p::seq< p::one<'.'>, PackageImportNames>
			>
		>
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
			p::opt< p::seq< p::one<','>, OptWs, TemplateParam> >
		>
	>
{
};

struct TemplateParams
	:
	p::if_must< p::one<'<'>, OptWs, TemplateParamsInner, OptWs, p::one<'>'> >
{
};

struct PossiblyTemplatedSymbolNoDisamb
	:
	p::seq< Name, p::opt<OptWs, TemplateParams> >
{};

struct PossiblyTemplatedSymbol
	:
	p::seq< Name,
		p::sor<
			p::seq< WsWrapped<p::string<':',':'>>, TemplateParams >,
			// If ::< Args > not succeded, then make sure the Name (at the start) isn't followed by the TemplateParams
			p::not_at<TemplateParams>
		>
	>
{};

struct Type
	: p::sor< PossiblyTemplatedSymbol, PossiblyTemplatedSymbolNoDisamb >
{
};

struct DeclType
	:
	p::sor<
		VarKeyword,
		ConstKeyword,
		Type
	>
{};

}
