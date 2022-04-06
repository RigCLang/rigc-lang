#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/TypeSystem/RefType.hpp>

#include <RigCInterpreter/VM.hpp>
#include <RigCInterpreter/Functions.hpp>

namespace rigc::vm
{

//////////////////////////////////////
void AddrType::postInitialize(Instance& vm_)
{
	auto& fn = vm_.universalScope().registerFunction(vm_, "get",
		Function{
			[](Instance& vm_, Function::Args& args_, size_t argCount_) -> OptValue
			{
				return vm_.allocateReference(args_[0].removeRef().removePtr());
			},
			{ { "self", wrap<RefType>(vm_.universalScope(), this->shared_from_this()) } },
			1
		}
	);
	fn.returnsRef = true;
	this->addMethod("get", &fn);
}


}
