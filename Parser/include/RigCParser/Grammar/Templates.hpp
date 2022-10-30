#pragma once

#include <RigCParser/RigCParserPCH.hpp>

#include <RigCParser/Grammar/Keywords.hpp>
#include <RigCParser/Grammar/Tokens.hpp>

namespace rigc {

struct TemplateDefParamKind
	: p::sor< TemplateTypenameKeyword, Name >
{
};


struct TemplateDefParamListElem
	:
	WsWrapped<
		Name, // name is a type constraint here, either a type itself or a genuine constraint, C++ concept like
		WsWrapped<p::one<':'>>,
		TemplateDefParamKind
	>
{
};

struct TemplateDefParamList
	:
	p::list_tail<TemplateDefParamListElem, p::one<','>>
{
};

struct TemplateDefPreamble
	:
	p::if_must<
		TemplateKeyword, OptWs,
		p::one<'<'>,
			WsWrapped<TemplateDefParamList>,
		p::one<'>'>
	>
{
};


}
