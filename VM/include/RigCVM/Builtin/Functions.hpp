#pragma once

#include RIGCVM_PCH

#include <RigCVM/Functions.hpp>
#include <RigCVM/Value.hpp>

namespace rigc::vm
{

struct Instance;

namespace builtin
{

OptValue print(Instance &vm_, Function::Args& args_, size_t argCount_);
OptValue typeOf(Instance &vm_, Function::Args& args_, size_t argCount_);
OptValue readInt(Instance &vm_, Function::Args& args_, size_t argCount_);
OptValue readFloat(Instance &vm_, Function::Args& args_, size_t argCount_);

}

}
