#include "VM/include/RigCVM/RigCVMPCH.hpp"

#include <RigCVM/Functions.hpp>

#include <RigCVM/VM.hpp>

namespace rigc::vm
{
///////////////////////////////////////////////////
auto Function::invoke(Instance& vm_, ArgSpan args_) const -> OptValue
{
	return vm_.executeFunction(*this, args_);
}
}
