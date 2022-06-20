#pragma once

#include RIGCVM_PCH

#include <RigCVM/TypeSystem/IType.hpp>
#include <RigCVM/Value.hpp>

namespace rigc::vm 
{
struct EnumValue : IType {
	EnumValue() = default;
	EnumValue(std::string name, Value val)
		: fieldName(std::move(name)),
			value(std::move(val))
			{}

	std::string fieldName;
	Value value;

	auto name() const -> std::string override { return value.type->name(); }
	auto size() const -> size_t override { return value.type->size(); }
	auto decay() const -> InnerType override { return nullptr; }
};
}
