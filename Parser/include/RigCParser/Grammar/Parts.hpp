#pragma once

#include RIGCPARSER_PCH

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
	: p::seq< opt_ws, p::one<'='>, opt_ws, AssignmentValue >
{
};

struct Initialization
	: p::seq< opt_ws, p::if_must< p::one<'='>, opt_ws, InitializerValue > >
{
};


}
