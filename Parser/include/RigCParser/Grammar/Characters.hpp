#pragma once

#include RIGCPARSER_PCH

namespace rigc
{

////////////////////// Whitespace /////////////////////////

struct whitespace
	: pegtl::plus< p::space >
{
};

struct Comment
	: p::seq< p::string<'/','/'>, p::until< p::eolf > >
{
};

struct ws
	: p::sor<
		Comment,
		p::seq< p::plus<whitespace>, p::opt<Comment> >
	>
{

};

// using ws = whitespace;

using OptWs = p::star<ws>;

template<typename... GrammarElems>
using WsWrapped = p::seq< OptWs, GrammarElems..., OptWs >;


///////////////////////////////////////////////
struct char_semicolon
	: p::one<';'>
{
};


struct Semicolon
	: p::seq< p::opt<whitespace>, p::one<';'> > 
{

};


}
