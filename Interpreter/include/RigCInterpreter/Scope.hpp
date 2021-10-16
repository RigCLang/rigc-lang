#pragma once

#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Value.hpp>

namespace rigc::vm
{

struct Scope
{
	std::map<std::string_view, Value> variables;
};

}