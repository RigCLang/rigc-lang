#pragma once

#include RIGCPARSER_PCH

namespace rigc
{

////////////////////// Whitespace /////////////////////////

struct whitespace
	: pegtl::plus< p::space >
{
};

using ws = whitespace;

using opt_ws = p::opt<ws>;

///////////////////////////////////////////////
struct char_semicolon
	: p::one<';'>
{
};


struct Semicolon
	: p::seq< p::opt<whitespace>, p::one<';'> > 
{

};

//////////////////////////////////////////////
struct TwoArgOper
	: p::sor<
		p::string< '+' >,
		p::string< '-' >,
		p::string< '*' >,
		p::string< '/' >,
		p::string< '%' >,
		p::string< '=','=' >,
		p::string< '!','=' >,
		p::string< '>','>' >,
		p::string< '<','<' >,
		p::string< '>' >,
		p::string< '<' >,
		p::string< '>','=' >,
		p::string< '<','=' >,
		p::string< '&' >,
		p::string< '|' >,
		p::string< '^' >
	>
{
};

struct ModOper2S
	: p::sor<
		p::string< '+','=' >,
		p::string< '-','=' >,
		p::string< '*','=' >,
		p::string< '/','=' >,
		p::string< '%','=' >,
		p::string< '>','>','=' >,
		p::string< '<','<','=' >,
		p::string< '&','=' >,
		p::string< '|','=' >,
		p::string< '^','=' >
	>
{
};

struct ModOper1S
	: p::sor<
		p::string< '+','+' >,
		p::string< '-','-' >
	>
{
};



}