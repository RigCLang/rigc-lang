#pragma once

#include RIGCVM_PCH

#include <RigCVM/Functions.hpp>
#include <RigCVM/Value.hpp>

namespace rigc::vm
{

struct Instance;

namespace builtin
{

auto print(Instance &vm_, Function::Args& args_, size_t argCount_) -> OptValue;
auto typeOf(Instance &vm_, Function::Args& args_, size_t argCount_) -> OptValue;
auto readInt(Instance &vm_, Function::Args& args_, size_t argCount_) -> OptValue;
auto readFloat(Instance &vm_, Function::Args& args_, size_t argCount_) -> OptValue;

}

}
