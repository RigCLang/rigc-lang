#pragma once

#include RIGCPARSER_PCH

#define DEFINE_KEYWORD(name, ...) \
	struct name##Keyword \
		: pegtl::keyword< __VA_ARGS__ > \
	{ }

namespace rigc
{

DEFINE_KEYWORD(From,		'f','r','o','m');
DEFINE_KEYWORD(Import,		'i','m','p','o', 'r', 't');
DEFINE_KEYWORD(Export,		'e','x','p','o','r', 't');
DEFINE_KEYWORD(Func,		'f','u','n','c');
DEFINE_KEYWORD(Var,			'v','a','r');
DEFINE_KEYWORD(Const,		'c','o','n','s','t');
DEFINE_KEYWORD(If,			'i','f');
DEFINE_KEYWORD(Else,		'e','l','s','e');
DEFINE_KEYWORD(While,		'w','h','i','l','e');
DEFINE_KEYWORD(Do,			'd','o');
DEFINE_KEYWORD(For,			'f','o','r');
DEFINE_KEYWORD(Ret,			'r','e','t');
DEFINE_KEYWORD(True,		't','r','u','e');
DEFINE_KEYWORD(False,		'f','a','l','s','e');
DEFINE_KEYWORD(Class,		'c','l','a','s','s');
DEFINE_KEYWORD(Override,	'o','v','e','r','r','i','d','e');
DEFINE_KEYWORD(Template,	't','e','m','p','l','a','t','e');
DEFINE_KEYWORD(TemplateTypename,	't','y','p','e','_','n','a','m','e');

}

#undef DEFINE_KEYWORD
