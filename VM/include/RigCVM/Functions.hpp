#pragma once

#include RIGCVM_PCH

#include <RigCVM/Value.hpp>
#include <RigCVM/ExtendedVariant.hpp>

namespace rigc::vm
{
struct Instance;
struct Scope;
class ClassType;

struct FunctionInstance
{
	rigc::ParserNode const* node;
	std::optional<TemplateArguments> templateArguments = std::nullopt;
};

struct RawFunctionInstance
{
	using Type = OptValue(Instance&, std::span<Value>);
	std::function< Type >	func;
	std::string				name = "<builtin>";
};

struct FunctionParam
{
	std::string_view		name;
	DeclType				type;
	rigc::ParserNode const*	typeNode = nullptr; // for unevaluated types in template functions
};

struct Function
{
	constexpr static size_t MAX_PARAMS = 16;
	using Params		= std::array<FunctionParam,	MAX_PARAMS>;
	using Args			= std::array<Value,			MAX_PARAMS>;

	using ParamSpan		= std::span<FunctionParam>;
	using ArgSpan		= std::span<Value>;

	using RuntimeFn		= FunctionInstance;

	using RawFn			= RawFunctionInstance;
	using ReturnType	= DeclType;

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
	bool				isConstructor = false;

	// TODO: workaround, remove this
	// once we have a proper explicit return type deduction for
	// func name -> Ref
	bool				returnsRef = false;

	auto invoke(Instance& vm_, ArgSpan args_) const -> OptValue;

	Function(Impl impl_, Params params_, size_t paramCount_)
		:
		impl(impl_),
		params(std::move(params_)),
		paramCount(paramCount_)
	{
	}

	Function(std::function<RawFn::Type> impl_, Params params_, size_t paramCount_)
		:
		impl(RawFn{ impl_ }),
		params(std::move(params_)),
		paramCount(paramCount_)
	{
	}

	auto runtimeImpl() const -> RuntimeFn const&
	{
		return impl.as<RuntimeFn>();
	}

	auto rawImpl() const -> std::function<RawFunctionInstance::Type> const&
	{
		return impl.as<RawFn>().func;
	}

	auto raw() -> RawFn& { return impl.as<RawFn>(); }
	auto raw() const -> RawFn const& { return impl.as<RawFn>(); }

	auto addr() const -> void const*
	{
		if (impl.is<RuntimeFn>()) {
			return this;
		} else {
			return rawImpl().target<RawFunctionInstance::Type*>();
		}
	}

	auto isRuntime() const -> bool
	{
		return impl.is<RuntimeFn>();
	}

	auto isRaw() const -> bool
	{
		return impl.is<RawFn>();
	}
};

using FunctionOverloads		= std::vector<Function*>;
using FunctionCandidates	= std::vector< std::pair<Scope const*, FunctionOverloads const*> >;
}
