#pragma once

#include RIGCPARSER_PCH

#include <RigCParser/Grammar/Parts.hpp>

namespace rigc
{

struct VariableDefinition
	: p::seq< DeclType, Ws, Name, p::opt< Initialization > >
{
};

}
