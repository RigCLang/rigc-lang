#pragma once

#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/TypeSystem/IType.hpp>
#include <RigCInterpreter/ExtendedVariant.hpp>
#include <RigCInterpreter/Value.hpp>

namespace rigc::vm
{

struct TemplateParameter
{
	std::string name;

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

	static TemplateParameter value(std::string name_, DeclType type, CompileTimeValue value={})
	{
		TemplateParameter param{std::move(name_)};
		param.variant = Value{std::move(type), std::move(value)};
		return param;
	}

	static TemplateParameter type(std::string name_, DeclType default_ = nullptr)
	{
		TemplateParameter param{std::move(name_)};
		param.variant = Type{std::move(default_)};
		return param;
	}

	bool isType() const { return variant.is<Type>(); }
	bool isValue() const { return variant.is<Value>(); }
	DeclType getDefaultType() const {
		return isType() ? variant.as<Type>().defaultType : nullptr;
	}
private:
	TemplateParameter(std::string name_)
		: name(std::move(name_))
	{
	}

	ExtendedVariant<Type, Value> variant;
};

using TemplateParameterValue = rigc::ParserNode const*;

}
