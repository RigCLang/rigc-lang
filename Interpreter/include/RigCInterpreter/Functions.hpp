#pragma once

#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Value.hpp>

namespace rigc::vm
{

struct Instance;

struct FunctionParam
{
	std::string_view	name;
	DeclType			type;
};

struct Function
{
	constexpr static size_t MAX_PARAMS = 128;
	using Params	= std::array<FunctionParam,	MAX_PARAMS>;
	using Args		= std::array<Value,			MAX_PARAMS>;

	using RuntimeFn	= rigc::ParserNode*;
	using RawFnSign	= OptValue(Instance&, Args&, size_t);
	using RawFn		= std::function<RawFnSign>;

	using Impl = std::variant<
			RuntimeFn,
			RawFn
		>;

	Impl		impl;
	Params		params;
	size_t		paramCount;

	OptValue invoke(Instance& vm_, Args& args_, size_t argCount_) const;

	Function(Impl impl_, Params params_, size_t paramCount_)
		:
		impl(impl_),
		params(std::move(params_)),
		paramCount(paramCount_)
	{
	}

	RuntimeFn runtimeImpl() const {
		return std::get<RuntimeFn>(impl);
	}

	RawFn rawImpl() const {
		return std::get<RawFn>(impl);
	}

	void const* addr() const {
		if (std::holds_alternative<RuntimeFn>(impl)) {
			return runtimeImpl();
		} else {
			return rawImpl().target<RawFnSign*>();
		}
	}
};

}
