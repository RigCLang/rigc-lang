#include "VM/include/RigCVM/RigCVMPCH.hpp"

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

	auto selfRefType = constructTemplateType<RefType>(vm_.universalScope(), this->shared_from_this());
	{
		Function::Params params;

		params[0].name = "self";
		params[0].type = selfRefType;

		params[1].name = "index";
		params[1].type = vm_.builtinTypes.Int32.shared();

		auto& fn = vm_.universalScope().registerOperator(vm_, "[]", Operator::Postfix,
			Function{
				[](Instance& vm_, Function::ArgSpan args_) -> OptValue
				{
					Value elem = args_[0].removeRef();
					elem.type = args_[0].type->decay();

					elem.data = (char*)elem.data + args_[1].view<int>() * elem.type->size();
					return vm_.allocateReference(elem);
				},
				params,
				2
			}
		);

		fn.returnType = constructTemplateType<RefType>(vm_.universalScope(), this->inner());
		this->addMethod("[]", &fn);
	}

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
				{ { "self", selfRefType } },
				1
			}
		);
		fn.returnType = constructTemplateType<AddrType>(vm_.universalScope(), this->inner());
		this->addMethod("data", &fn);
	}

	// "size" method
	{
		auto& fn = vm_.universalScope().registerFunction(vm_, "size",
			Function{
				[this](Instance& vm_, Function::ArgSpan args_) -> OptValue
				{
					auto val = args_[0].removeRef();
					return vm_.allocateOnStack(
							vm_.builtinTypes.Int32.shared(),
							int(args[1].as<int>())
						);
				},
				{ { "self", selfRefType } },
				1
			}
		);
		fn.returnType = vm_.builtinTypes.Int32.shared();
		this->addMethod("size", &fn);
	}
}
}
