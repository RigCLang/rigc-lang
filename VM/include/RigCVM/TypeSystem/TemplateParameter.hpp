#pragma once

#include RIGCVM_PCH

#include <RigCVM/TypeSystem/IType.hpp>
#include <RigCVM/ExtendedVariant.hpp>
#include <RigCVM/Value.hpp>

namespace rigc::vm
{

struct TemplateParameter
{
	String name;

	struct Type
	{
		DeclType defaultType;
		// TODO: add constraints
	};
	struct Value
	{
		DeclType				type;
		Opt<CompileTimeValue>	defaultValue;
	};

	static auto value(String name_, DeclType type, CompileTimeValue value={}) -> TemplateParameter
	{
		TemplateParameter param{std::move(name_)};
		param.variant = Value{std::move(type), std::move(value)};
		return param;
	}

	static auto type(String name_, DeclType default_ = nullptr) -> TemplateParameter
	{
		TemplateParameter param{std::move(name_)};
		param.variant = Type{std::move(default_)};
		return param;
	}

	auto isType() const -> bool { return variant.is<Type>(); }
	auto isValue() const -> bool { return variant.is<Value>(); }
	auto getDefaultType() const -> DeclType
	{
		return isType() ? variant.as<Type>().defaultType : nullptr;
	}

private:
	TemplateParameter(String name_)
		: name(std::move(name_))
	{
	}

	ExtendedVariant<Type, Value> variant;
};

using TemplateParameterValue = rigc::ParserNode const*;
}
