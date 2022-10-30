#pragma once

#include <RigCParser/RigCParserPCH.hpp>

namespace rigc
{

using ParserNode	= p::parse_tree::node;
using ParserNodePtr	= std::unique_ptr< ParserNode >;

auto parse(p::file_input<> &in) -> ParserNodePtr;

}
