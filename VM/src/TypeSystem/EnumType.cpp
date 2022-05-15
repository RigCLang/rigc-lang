#include RIGCVM_PCH

#include <RigCVM/TypeSystem/EnumType.hpp>
#include <RigCVM/TypeSystem/EnumValue.hpp>
#include <RigCVM/TypeSystem/WrapperType.hpp>
#include <RigCVM/TypeSystem/RefType.hpp>
#include <RigCVM/Alloc.hpp>

namespace rigc::vm
{

auto EnumType::add(DataMember mem, OptValue const& val) -> void {
	_size = mem.type->size();

	if(!val) {
		throw std::runtime_error("Automatic enum indexing not implemented yet.");
	}

	auto staticEnumValue = allocateStaticValue(this->shared_from_this(), EnumValue(mem.name, *val));
	auto const[it, insertionHappened] = fields.try_emplace( mem.name, staticEnumValue );

	if(!insertionHappened)
		throw std::runtime_error(fmt::format("Member {} already present in the enum.\n", mem.name));
}


auto EnumType::postInitialize(Instance& vm) -> void {

	// assignment operator
	{
		auto params = Function::Params {{
			{ "self", this->shared_from_this() },
			{ "rhs", this->shared_from_this() }
		}};

		auto& fn = vm.universalScope().registerOperator(
			vm,
			"=",
			Operator::Infix,
			Function{
				[](Instance& vm_, Function::Args& args_, size_t argCount_) -> OptValue
				{
					auto self = args_[0].safeRemoveRef();
					auto const rhsEnumValue = args_[1].safeRemoveRef().view<EnumValue>();
				
					self.view<EnumValue>() = rhsEnumValue;

					return vm_.allocateReference(self);
				},
				std::move(params),
				2
			}
		);
	} // assignment operator

	// equality operator
	{
		auto params = Function::Params {{
			{ "self", this->shared_from_this() },
			{ "rhs", this->shared_from_this() }
		}};

		auto& fn = vm.universalScope().registerOperator(
			vm,
			"==",
			Operator::Infix,
			Function{
				[](Instance& vm_, Function::Args& args_, size_t argCount_) -> OptValue
				{
					auto const selfEnumValue = args_[0].safeRemoveRef().view<EnumValue>();
					auto const rhsEnumValue = args_[1].safeRemoveRef().view<EnumValue>();

					return vm_.allocateOnStack("Bool", selfEnumValue.fieldName == selfEnumValue.fieldName);
				},
				std::move(params),
				2
			}
		);
	} // equality operator
}

}
