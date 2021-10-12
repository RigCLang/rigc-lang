#pragma once

#include RIGC_PCH

#include <RigC/Grammar/Keywords.hpp>
#include <RigC/Grammar/Tokens.hpp>
#include <RigC/Grammar/Expression.hpp>

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
	: p::seq< opt_ws, p::one<'='>, opt_ws, InitializerValue >
{
};


}