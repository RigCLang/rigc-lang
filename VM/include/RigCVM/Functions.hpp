#pragma once

#include RIGCVM_PCH

#include <RigCVM/Value.hpp>
#include <RigCVM/ExtendedVariant.hpp>

namespace rigc::vm
{

struct Instance;
class ClassType;

using TemplateArgument	= ExtendedVariant<int, DeclType>;
using TemplateArguments	= std::map<std::string, TemplateArgument, std::less<>>;

struct FunctionInstance
{
	rigc::ParserNode const* node;
	std::optional<TemplateArguments> templateArguments = std::nullopt;
};

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

	using RuntimeFn		= FunctionInstance;
	using RawFnSign		= OptValue(Instance&, Args&, size_t);
	using RawFn			= std::function<RawFnSign>;
	using ReturnType	= std::optional<DeclType>;

	using Impl = ExtendedVariant<
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

	auto runtimeImpl() const -> RuntimeFn const&
	{
		return impl.as<RuntimeFn>();
	}

	auto rawImpl() const -> RawFn
	{
		return impl.as<RawFn>();
	}

	auto addr() const -> void const*
	{
		if (impl.is<RuntimeFn>()) {
			return runtimeImpl().node;
		} else {
			return rawImpl().target<RawFnSign*>();
		}
	}

	auto isRuntime() const  -> bool
	{
		return impl.is<RuntimeFn>();
	}

	auto isRaw() const  -> bool
	{
		return impl.is<RawFn>();
	}
};

using FunctionOverloads = std::vector<Function*>;

}
