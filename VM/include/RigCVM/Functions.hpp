#pragma once

#include RIGCVM_PCH

#include <RigCVM/Value.hpp>

namespace rigc::vm
{

struct Instance;
class ClassType;

struct FunctionParam
{
	std::string_view	name;
	DeclType			type;
};

struct Function
{
	constexpr static size_t MAX_PARAMS = 128;
	using Params		= std::array<FunctionParam,	MAX_PARAMS>;
	using Args			= std::array<Value,			MAX_PARAMS>;

	using RuntimeFn		= rigc::ParserNode const*;
	using RawFnSign		= OptValue(Instance&, Args&, size_t);
	using RawFn			= std::function<RawFnSign>;
	using ReturnType	= std::optional<DeclType>;

	using Impl = std::variant<
			RuntimeFn,
			RawFn
		>;

	Impl				impl;
	Params				params;
	size_t				paramCount;
	ReturnType			returnType;
	bool				variadic = false;
	IType*				outerType = nullptr;

	// TODO: workaround, remove this
	// once we have a proper explicit return type deduction for
	// func name -> Ref
	bool				returnsRef = false;

	OptValue invoke(Instance& vm_, Args& args_, size_t argCount_) const;

	Function(Impl impl_, Params params_, size_t paramCount_)
		:
		impl(impl_),
		params(std::move(params_)),
		paramCount(paramCount_)
	{
	}

	auto runtimeImpl() const  -> RuntimeFn
	{
		return std::get<RuntimeFn>(impl);
	}

	auto rawImpl() const  -> RawFn
	{
		return std::get<RawFn>(impl);
	}

	auto addr() const -> void const* 
	{
		if (std::holds_alternative<RuntimeFn>(impl)) {
			return runtimeImpl();
		} else {
			return rawImpl().target<RawFnSign*>();
		}
	}

	auto isRuntime() const  -> bool
	{
		return std::holds_alternative<RuntimeFn>(impl);
	}

	auto isRaw() const  -> bool
	{
		return std::holds_alternative<RawFn>(impl);
	}
};

using FunctionOverloads = std::vector<Function*>;

}
