#include RIGCVM_PCH

#include <RigCVM/TypeSystem/RefType.hpp>

#include <RigCVM/VM.hpp>
#include <RigCVM/Functions.hpp>

namespace rigc::vm
{
//////////////////////////////////////
auto RefType::postInitialize(Instance& vm_) -> void
{
	// Super::postInitialize(vm_);

	auto self = this->shared_from_this();

	// rhs is lvalue reference
	{
		auto params = Function::Params{{
			{ "self", self },
			{ "rhs", self }
		}};
		auto& fn	= vm_.scopeOf(this).registerFunction(vm_, "construct", Function{
			[](Instance &vm_, Function::ArgSpan args_) -> OptValue
			{
				auto self = args_[0];
				std::memcpy(
						self.data,
						args_[1].data,
						self.type->size()
					);
				return std::nullopt;
			},
			std::move(params), 2
		});
		fn.isConstructor = true;
		this->addMethod("construct", &fn);
	}


	// Setup template arguments
	assert((args.size() == 1) && "RefType::postInitialize: AddrType must have 1 template argument");
}

//////////////////////////////////////
auto AddrType::postInitialize(Instance& vm_) -> void
{
	Super::postInitialize(vm_);

	assert((args.size() == 1) && "AddrType::postInitialize: AddrType must have 1 template argument");

	auto refToSelf = constructTemplateType<RefType>(vm_.universalScope(), this->shared_from_this());

	// get method
	{
		auto& fn = vm_.universalScope().registerFunction(vm_, "get",
			Function{
				[](Instance& vm_, Function::ArgSpan args_) -> OptValue
				{
					return vm_.allocateReference(args_[0].removeRef().removePtr());
				},
				{ { "self", refToSelf } },
				1
			}
		);
		fn.returnsRef = true;
		this->addMethod("get", &fn);
	}

	// plus operator
	{
		Function::Params params;


		params[0] = { "self", this->shared_from_this() };
		params[1] = { "rhs", vm_.findType("Int32")->shared_from_this() };

		auto& fn = vm_.universalScope().registerOperator(vm_, "+", Operator::Infix,
			Function{
				[](Instance& vm_, Function::ArgSpan args_) -> OptValue
				{
					auto self = args_[0].safeRemoveRef();
					return vm_.allocateOnStack<void const*>(
							self.type,
							self.view<char const*>() + self.type->decay()->size() * args_[1].view<int32_t>()
						);
				},
				std::move(params),
				2
			}
		);

		fn.returnType = this->shared_from_this();
	}

	// assign operator
	{
		Function::Params params;

		params[0] = { "self", refToSelf };
		params[1] = { "rhs", this->shared_from_this() };

		auto& fn = vm_.universalScope().registerOperator(vm_, "=", Operator::Infix,
			Function{
				[](Instance& vm_, Function::ArgSpan args_) -> OptValue
				{
					auto self = args_[0].safeRemoveRef();
					self.view<void*>() = args_[1].view<void*>();
					return args_[0];
				},
				std::move(params),
				2
			}
		);

		fn.returnType = refToSelf;
	}
}
}
