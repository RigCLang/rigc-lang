#pragma once

#include RIGCPARSER_PCH

namespace rigc
{

////////////////////// Whitespace /////////////////////////

struct Whitespace
	: pegtl::plus< p::space >
{
};

struct Comment
	: p::seq< p::string<'/','/'>, p::until< p::eolf > >
{
};

struct Ws
	: p::sor<
		Comment,
		p::seq< p::plus<Whitespace>, p::opt<Comment> >
	>
{
};

// using Rs = Whitespace;

using OptWs = p::star<Ws>;

template<typename... GrammarElems>
using WsWrapped = p::seq< OptWs, GrammarElems..., OptWs >;


///////////////////////////////////////////////
struct char_semicolon
	: p::one<';'>
{
};


struct Semicolon
	: p::seq< p::opt<Whitespace>, p::one<';'> > 
{
};


}
