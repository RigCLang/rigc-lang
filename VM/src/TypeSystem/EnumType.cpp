#include RIGCVM_PCH

#include <RigCVM/TypeSystem/EnumType.hpp>
#include <RigCVM/TypeSystem/EnumValue.hpp>
#include <RigCVM/TypeSystem/WrapperType.hpp>
#include <RigCVM/TypeSystem/RefType.hpp>
#include <RigCVM/Alloc.hpp>

namespace rigc::vm
{
auto EnumType::add(DataMember mem, OptValue const& val) -> void
{
	_size = underlyingType->size();

	if(!val) {
		throw std::runtime_error("Automatic enum indexing not implemented yet.");
	}

	// TODO: use a proper constructor
	auto staticValue = allocateStaticValue( this->shared_from_this(), val->data, 4 );
	auto const[it, insertionHappened] = fields.try_emplace( mem.name, staticValue );

	if(!insertionHappened)
		throw std::runtime_error(fmt::format("Member {} already present in the enum.\n", mem.name));
}


auto EnumType::postInitialize(Instance& vm) -> void
{
	// assignment operator
	{
		auto selfRef = constructTemplateType<RefType>(vm.scopeOf(nullptr), this->shared_from_this());
		auto params = Function::Params {{
			{ "self", selfRef },
			{ "rhs", this->shared_from_this() }
		}};

		auto& fn = vm.universalScope().registerOperator(
			vm,
			"=",
			Operator::Infix,
			Function{
				[](Instance& vm_, Function::ArgSpan args_) -> OptValue
				{
					auto const& self	= args_[0].removeRef();
					auto const& rhs		= args_[1];
					auto enumType		= self.type->as<EnumType>();
					auto overloads		= vm_.universalScope().findOperator("=", Operator::Infix);
					auto selfRef		= vm_.allocateReference( Value{ enumType->underlyingType, self.data } );

					auto types = FunctionParamTypes{
							selfRef.type,
							enumType->underlyingType
						};
					auto fn = findOverload(*overloads, viewArray(types, 0, 2));

					if (!fn) {
						throw std::runtime_error("No overload found for == operator.");
					}

					auto args = Function::Args{
						selfRef,
						Value{ enumType->underlyingType, rhs.data }
					};
					return vm_.executeFunction(*fn, args);
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
				[](Instance& vm_, Function::ArgSpan args_) -> OptValue
				{
					auto const& self	= args_[0];
					auto const& rhs		= args_[1];
					auto enumType		= args_[0].type->as<EnumType>();
					auto overloads		= vm_.universalScope().findOperator("==", Operator::Infix);

					auto types = FunctionParamTypes{
						enumType->underlyingType,
						enumType->underlyingType
					};
					auto fn = findOverload(*overloads, viewArray(types, 0, 2));

					if (!fn) {
						throw std::runtime_error("No overload found for == operator.");
					}

					auto args = Function::Args{
						Value{ enumType->underlyingType, self.data },
						Value{ enumType->underlyingType, rhs.data }
					};

					return vm_.executeFunction(*fn, args);
				},
				std::move(params),
				2
			}
		);
	} // equality operator
}
}
