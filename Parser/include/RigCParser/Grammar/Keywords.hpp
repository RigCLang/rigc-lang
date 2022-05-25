#pragma once

#include RIGCPARSER_PCH

#define DEFINE_KEYWORD(structName, keywordName) \
	struct structName##Keyword \
		: TAO_PEGTL_KEYWORD( keywordName ) \
	{ }

namespace rigc
{

DEFINE_KEYWORD(From,		"from");
DEFINE_KEYWORD(Import,		"import");
DEFINE_KEYWORD(Export,		"export");
DEFINE_KEYWORD(Func,		"func");
DEFINE_KEYWORD(Var,			"var");
DEFINE_KEYWORD(Const,		"const");
DEFINE_KEYWORD(If,			"if");
DEFINE_KEYWORD(Else,		"else");
DEFINE_KEYWORD(While,		"while");
DEFINE_KEYWORD(Do,			"do");
DEFINE_KEYWORD(For,			"for");
DEFINE_KEYWORD(Ret,			"ret");
DEFINE_KEYWORD(True,		"true");
DEFINE_KEYWORD(False,		"false");
DEFINE_KEYWORD(Class,		"class");
DEFINE_KEYWORD(Union,		"union");
DEFINE_KEYWORD(Enum,		"enum");
DEFINE_KEYWORD(Override,	"override");
DEFINE_KEYWORD(Template,	"template");
DEFINE_KEYWORD(TemplateTypename,	"type_name");
DEFINE_KEYWORD(Of,	"of");
DEFINE_KEYWORD(As,	"as");

}

#undef DEFINE_KEYWORD
