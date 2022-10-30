#pragma once

#include <RigCParser/RigCParserPCH.hpp>

#include <RigCParser/Grammar/Parts.hpp>

namespace rigc
{

struct VariableDefinition
	: p::seq< DeclType, Ws, Name, p::opt< Initialization > >
{
};

}
