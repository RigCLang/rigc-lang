#pragma once

#include RIGC_PCH

#define DEFINE_KEYWORD(name, ...) \
	struct name##Keyword \
		: pegtl::keyword< __VA_ARGS__ > \
	{ }

namespace rigc
{

DEFINE_KEYWORD(From,	'f','r','o','m');
DEFINE_KEYWORD(Import,	'i','m','p','o', 'r', 't');
DEFINE_KEYWORD(Func,	'f','u','n','c');
DEFINE_KEYWORD(Var,		'v','a','r');
DEFINE_KEYWORD(Const,	'c','o','n','s','t');
DEFINE_KEYWORD(If,		'i','f');
DEFINE_KEYWORD(Else,	'e','l','s','e');
DEFINE_KEYWORD(While,	'w','h','i','l','e');
DEFINE_KEYWORD(Do,		'd','o');
DEFINE_KEYWORD(For,		'f','o','r');

}

#undef DEFINE_KEYWORD