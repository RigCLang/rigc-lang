#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/TypeSystem/ArrayType.hpp>
#include <RigCInterpreter/TypeSystem/RefType.hpp>

#include <RigCInterpreter/VM.hpp>

namespace rigc::vm
{

//////////////////////////////////////
void ArrayType::postInitialize(Instance& vm_)
{
	// "data" method
	{
		auto& fn = vm_.universalScope().registerFunction(vm_, "data",
			Function{
				[](Instance& vm_, Function::Args& args_, size_t argCount_) -> OptValue
				{
					Value ptr = args_[0].removeRef();
					ptr.data = &ptr.data;
					ptr.type = wrap<AddrType>(vm_.universalScope(), ptr.type->decay());

					return ptr;
				},
				{ { "self", wrap<RefType>(vm_.universalScope(), this->shared_from_this()) } },
				1
			}
		);
		this->addMethod("data", &fn);
	}

	// "size" method
	{
		auto& fn = vm_.universalScope().registerFunction(vm_, "size",
			Function{
				[](Instance& vm_, Function::Args& args_, size_t argCount_) -> OptValue
				{
					auto val = args_[0].removeRef();
					return vm_.allocateOnStack(
							vm_.findType("Int32")->shared_from_this(),
							int(val.type->size())
						);
				},
				{ { "self", wrap<RefType>(vm_.universalScope(), this->shared_from_this()) } },
				1
			}
		);
		this->addMethod("size", &fn);
	}
}

}
