#pragma once

#include RIGC_PCH

#include <RigC/Grammar/Parts.hpp>

namespace rigc
{

struct VariableDefinition
	: p::seq<DeclType, ws, Name, p::opt< Initialization > >
{
};

}