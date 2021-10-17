#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Functions.hpp>

#include <RigCInterpreter/VM.hpp>

namespace rigc::vm
{

///////////////////////////////////////////////////
OptValue Function::invoke(Instance& vm_, Args& args_, size_t argCount_) const
{
	return vm_.executeFunction(*this, args_, argCount_);
}

}