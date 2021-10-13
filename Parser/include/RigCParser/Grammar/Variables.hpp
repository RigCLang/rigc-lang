#pragma once

#include RIGCPARSER_PCH

#include <RigCParser/Grammar/Parts.hpp>

namespace rigc
{

struct VariableDefinition
	: p::seq<DeclType, ws, Name, p::opt< Initialization > >
{
};

}