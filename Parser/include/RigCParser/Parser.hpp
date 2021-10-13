#pragma once

#include RIGCPARSER_PCH

namespace rigc
{

using ParserNode	= p::parse_tree::node;
using ParserNodePtr	= std::unique_ptr< ParserNode >;

ParserNodePtr parse(p::file_input<> &in);

}