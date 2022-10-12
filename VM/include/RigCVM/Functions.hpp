#pragma once

#include RIGCVM_PCH

#include <RigCVM/Value.hpp>
#include <RigCVM/Helper/ExtendedVariant.hpp>

namespace rigc::vm
{
struct Instance;
struct Scope;
class ClassType;

struct FunctionInstance
{
	rigc::ParserNode const* node;
	Opt<TemplateArguments> templateArguments = std::nullopt;
};

struct RawFunctionInstance
{
	using Type = OptValue(Instance&, Span<Value>);
	Func< Type >	func;
	String			name = "<builtin>";
};

struct FunctionParam
{
	StringView				name;
	DeclType				type;
	rigc::ParserNode const*	typeNode = nullptr; // for unevaluated types in template functions
};

struct Function
{
	constexpr static size_t MAX_PARAMS = 16;
	using Params		= Array<FunctionParam,	MAX_PARAMS>;
	using Args			= Array<Value,			MAX_PARAMS>;

	using ParamSpan		= Span<FunctionParam>;
	using ArgSpan		= Span<Value>;

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

	Function(Func<RawFn::Type> impl_, Params params_, size_t paramCount_)
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

	auto rawImpl() const -> Func<RawFunctionInstance::Type> const&
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

using FunctionOverloads		= DynArray< Function* >;
using FunctionCandidates	= DynArray< Pair<Scope const*, FunctionOverloads const*> >;
}
