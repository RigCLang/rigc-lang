#include RIGCVM_PCH

#include <RigCVM/TypeSystem/IType.hpp>
#include <RigCVM/TypeSystem/ClassType.hpp>
#include <RigCVM/TypeSystem/RefType.hpp>
#include <RigCVM/TypeSystem/WrapperType.hpp>
#include <RigCVM/Functions.hpp>

namespace rigc::vm
{

//////////////////////////////////////
auto IType::addMethod(StringView name_, Function* func_) -> void
{
	this->methods[name_].push_back(func_);
	func_->outerType = this;
}

///////////////////////////////////////////
auto IType::constructors() const
	-> FunctionOverloads const*
{
	auto ctorsIt = methods.find("construct");
	if (ctorsIt == methods.end())
		return nullptr;

	return &ctorsIt->second;
}


//////////////////////////////////////
auto IType::postInitialize(Instance& vm_) -> void
{
	bool hasCopyCtor = false;

	auto paramTypes = FunctionParamTypes{

	};
	auto ctors = this->constructors();
	if (ctors) {

		auto ov = findOverload(*ctors, viewArray(paramTypes), true);
		if (ov) {
			hasCopyCtor = true;
		}
	}

	if (!hasCopyCtor)
	{
		auto& scope	= vm_.scopeOf(this);

		auto selfRef = constructTemplateType<RefType>(vm_.universalScope(), this->shared_from_this());


		// rhs is lvalue reference
		{
			auto params = Function::Params{{
				{ "self", selfRef },
				{ "rhs", selfRef }
			}};
			auto& fn	= scope.registerFunction(vm_, "construct", Function{
				[](Instance &vm_, Function::ArgSpan args_) -> OptValue
				{
					auto self = args_[0].removeRef();
					std::memcpy(
							self.data,
							args_[1].removeRef().data,
							self.type->size()
						);
					return std::nullopt;
				},
				std::move(params), 2
			});
			fn.isConstructor = true;
			this->addMethod("construct", &fn);
		}

		// rhs is lvalue
		{
			auto params = Function::Params{{
				{ "self", selfRef },
				{ "rhs", this->shared_from_this() }
			}};

			auto& fn	= scope.registerFunction(vm_, "construct", Function{
				RawFunctionInstance {
					[](Instance &vm_, Function::ArgSpan args_) -> OptValue
					{
						auto self = args_[0].removeRef();
						std::memcpy(
								self.data,
								args_[1].data,
								self.type->size()
							);
						return std::nullopt;
					}, fmt::format("{} :: construct({}, {})", this->name(), selfRef->name(), this->name())
				},
				std::move(params), 2
			});
			fn.isConstructor = true;
			this->addMethod("construct", &fn);
		}
	}
}


}
