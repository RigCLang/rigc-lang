#pragma once

#include <RigCParser/RigCParserPCH.hpp>

#include <RigCParser/Grammar/Keywords.hpp>
#include <RigCParser/Grammar/Tokens.hpp>
#include <RigCParser/Grammar/Expression.hpp>
#include <RigCParser/Grammar/Characters.hpp>

namespace rigc
{

struct AssignmentValue
	: Expression
{
};

struct InitializerValue
	: AssignmentValue
{
};

struct Assignment
	: p::seq< OptWs, p::one<'='>, OptWs, AssignmentValue >
{
};

struct Initialization
	: p::seq< OptWs, p::if_must< p::one<'='>, OptWs, InitializerValue > >
{
};


}
