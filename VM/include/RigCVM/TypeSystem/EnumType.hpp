#pragma once

#include RIGCVM_PCH

#include <RigCVM/TypeSystem/Shared/DataMember.hpp>
#include <RigCVM/TypeSystem/StructuralType.hpp>
#include <RigCVM/Scope.hpp>
#include <RigCVM/VM.hpp>

namespace rigc::vm
{

class EnumType

	:
	public StructuralType
{
public:
	DeclType underlyingType;
	std::unordered_map<std::string, Value> fields;

	auto add(DataMember mem, OptValue const& val) -> void;
};

}
