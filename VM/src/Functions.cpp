#include RIGCVM_PCH

#include <RigCVM/Functions.hpp>

#include <RigCVM/VM.hpp>

namespace rigc::vm
{

///////////////////////////////////////////////////
OptValue Function::invoke(Instance& vm_, Args& args_, size_t argCount_) const
{
	return vm_.executeFunction(*this, args_, argCount_);
}

}
