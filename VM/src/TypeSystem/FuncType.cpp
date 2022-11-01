#include "VM/include/RigCVM/RigCVMPCH.hpp"

#include <RigCVM/TypeSystem/FuncType.hpp>

#include <RigCVM/VM.hpp>
#include <RigCVM/Functions.hpp>
#include <RigCVM/Type.hpp>
#include <RigCVM/TypeSystem/RefType.hpp>
#include <RigCVM/Value.hpp>

namespace rigc::vm
{
//////////////////////////////////////
auto FuncType::postInitialize(Instance& vm_) -> void
{
	Super::postInitialize(vm_);

	{
		// Func<R, Params...>
		// Copy params starting from index 1
		// Operator() has following params:
		// (self: Ref< Func<R, Params...> >, params...)
		Function::Params params;

		params[0].name = "self";
		params[0].type = constructTemplateType<RefType>(vm_.universalScope(), this->shared_from_this());

		for (size_t i = 1; i < this->args.size(); ++i)
		{
			params[i].name = fmt::format("arg{}", i);
			params[i].type = this->args[i].as<DeclType>();
		}

		auto& fn = vm_.universalScope().registerOperator(vm_, "()", Operator::Postfix,
			Function{
				[](Instance& vm_, Function::ArgSpan args_) -> OptValue
				{
					return vm_.executeFunction(*args_[0].removeRef().view<Function*>(), args_.subspan(1));
				},
				params,
				args.size() // correct because self param compensates for the return type at args[0]
			}
		);

		fn.returnType = args[0].as<DeclType>();
		this->addMethod("()", &fn);
	}
}

//////////////////////////////////////
auto FuncType::name() const -> String
{
	auto ret = String("Func<" + args[0].as<DeclType>()->name());
	for (size_t i = 1; i < args.size(); ++i) {
		ret += ", ";
		ret += args[i].as<DeclType>()->name();
	}
	ret += ">";
	return ret;
}

//////////////////////////////////////
auto FuncType::hashWrapped(DeclType const& returnType, Span<DeclType> args) -> std::size_t
{
	auto ret = String("Func<" + returnType->name());
	for (auto& a : args) {
		ret += ", ";
		ret += a->name();
	}
	ret += ">";
	return std::hash<String>{}(ret);
}

//////////////////////////////////////
auto constructFunctionType(Scope& ownerScope_, Span<DeclType> args_) -> MutDeclType
{
	auto hash = FuncType::hashWrapped(args_[0], args_.subspan(1));

	if (auto type = ownerScope_.types.find(hash))
		return type;

	auto wrapper = std::make_shared<FuncType>(args_);
	ownerScope_.addType(wrapper);
	return wrapper;
}

}
