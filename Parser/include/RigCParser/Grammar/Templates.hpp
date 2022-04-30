#pragma once

#include RIGCPARSER_PCH

#include <RigCParser/Grammar/Keywords.hpp>
#include <RigCParser/Grammar/Tokens.hpp>

namespace rigc {

struct TemplateParamKind
	: p::sor<TemplateTypenameKeyword, Name>
{
};

struct TemplateParamListElem
	: 
	p::seq<WsWrapped<Name, WsWrapped<p::one<':'>>, TemplateParamKind>>
// name is a type constraint here, either a type itself or a genuine constraint, C++ concept like
{
};

struct TemplatePreamble
	:
	p::if_must<
		TemplateKeyword,
		WsWrapped<p::one<'<'>>,
			p::list_tail<TemplateParamListElem, p::one<','>>,
		WsWrapped<p::one<'>'>>
	>
{
};


}
