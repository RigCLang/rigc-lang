#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Functions.hpp>

#include <RigCInterpreter/VM.hpp>

namespace rigc::vm
{

///////////////////////////////////////////////////
OptValue Function::invoke(Instance& vm_, Args& args_, size_t argCount_) const
{
	// Raw function:
	if (std::holds_alternative<RawFn>(impl))
	{
		auto const& fn = std::get<RawFn>(impl);
		return fn(vm_, args_, argCount_);
	}

	// Runtime function:
	auto const& fn = *std::get<RuntimeFn>(impl);
	return vm_.executeFunction(fn, args_, argCount_);
}

}