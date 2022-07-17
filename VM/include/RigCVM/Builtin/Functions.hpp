#pragma once

#include RIGCVM_PCH

#include <RigCVM/Functions.hpp>
#include <RigCVM/Value.hpp>

namespace rigc::vm
{

struct Instance;

namespace builtin
{

auto print(Instance &vm_, Function::ArgSpan args_) -> OptValue;
auto typeOf(Instance &vm_, Function::ArgSpan args_) -> OptValue;
auto readInt(Instance &vm_, Function::ArgSpan args_) -> OptValue;
auto readFloat(Instance &vm_, Function::ArgSpan args_) -> OptValue;

auto allocateMemory(Instance &vm_, Function::ArgSpan args_) -> OptValue;
auto freeMemory(Instance &vm_, Function::ArgSpan args_) -> OptValue;

}

}
