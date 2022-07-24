#include RIGCVM_PCH

#include <RigCVM/TypeSystem/ArrayType.hpp>
#include <RigCVM/TypeSystem/RefType.hpp>

#include <RigCVM/VM.hpp>

namespace rigc::vm
{
//////////////////////////////////////
auto ArrayType::postInitialize(Instance& vm_) -> void
{
	Super::postInitialize(vm_);

	// Setup template arguments:
	assert((args.size() == 2) && "ArrayType::postInitialize: ArrayType must have 2 template arguments");

	// "data" method
	{
		auto& fn = vm_.universalScope().registerFunction(vm_, "data",
			Function{
				[](Instance& vm_, Function::ArgSpan args_) -> OptValue
				{
					Value firstElem = args_[0].removeRef();
					firstElem.type = args_[0].type->decay();

					return vm_.allocatePointer(firstElem);
				},
				{ { "self", constructTemplateType<RefType>(vm_.universalScope(), this->shared_from_this()) } },
				1
			}
		);
		this->addMethod("data", &fn);
	}

	// "size" method
	{
		auto& fn = vm_.universalScope().registerFunction(vm_, "size",
			Function{
				[](Instance& vm_, Function::ArgSpan args_) -> OptValue
				{
					auto val = args_[0].removeRef();
					return vm_.allocateOnStack(
							vm_.findType("Int32")->shared_from_this(),
							int(val.type->size())
						);
				},
				{ { "self", constructTemplateType<RefType>(vm_.universalScope(), this->shared_from_this()) } },
				1
			}
		);
		this->addMethod("size", &fn);
	}
}
}
