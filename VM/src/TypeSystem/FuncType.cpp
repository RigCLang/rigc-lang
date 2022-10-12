#include RIGCVM_PCH

#include <RigCVM/TypeSystem/FuncType.hpp>

#include <RigCVM/VM.hpp>
#include <RigCVM/Functions.hpp>
#include <RigCVM/Type.hpp>
#include <RigCVM/Value.hpp>

namespace rigc::vm
{
//////////////////////////////////////
auto FuncType::postInitialize(Instance& vm_) -> void
{
	Super::postInitialize(vm_);
	// Function::Params params;
	// for (size_t i = 0; i < parameters.size(); ++i) {
	// 	params[i].type = parameters[i];
	// 	params[i].name
	// }

	// auto& fn = vm_.universalScope().registerOperator(vm_, "()", Operator::Postfix,
	// 	Function{
	// 		[](Instance& vm_, Function::Args& args_, size_t argCount_) -> OptValue
	// 		{
	// 			Function::Args innerArgs;
	// 			for (size_t i = 1; i < argCount_; ++i) {
	// 				innerArgs[i - 1] = args_[i];
	// 			}

	// 			return vm_.executeFunction(args_[0].view<Function*>(), innerArgs, argCount_ - 1);
	// 		},
	// 		{ { "self", constructTemplateType<RefType>(vm_.universalScope(), this->shared_from_this()) } },
	// 		1
	// 	}
	// );
	// fn.returnsRef = true;
	// this->addMethod("()", &fn);
}

//////////////////////////////////////
auto FuncType::name() const -> String
{
	if (!result && parameters.empty())
		return String(builtin_types::OverloadedFunction);

	String ret = "Func<" + result->name();
	for (auto const& param : parameters) {
		ret += ", ";
		ret += param->name();
	}
	ret += ">";
	return ret;
}

//////////////////////////////////////
auto MethodType::name() const -> String
{
	if (!result && parameters.empty())
		return String(builtin_types::OverloadedMethod);

	String ret = "Method<" + classType->name() + ", " + result->name();
	for (auto const& param : parameters) {
		ret += ", ";
		ret += param->name();
	}
	ret += ">";
	return ret;
}

//////////////////////////////////////
auto allocateMethodOverloads(Instance& vm_, Value self_, FunctionOverloads const* overloads_) -> Value
{
	using PtrType = void const*;

	PtrType buf[ 2 ];

	buf[0] = overloads_;
	buf[1] = self_.data;

	auto type = vm_.findType(builtin_types::OverloadedMethod);
	return vm_.allocateOnStack(type->shared_from_this(), (void const*)buf, sizeof(buf));
}
}
