#pragma once

#include <RigCVM/RigCVMPCH.hpp>

#include <RigCVM/TypeSystem/Shared/DataMember.hpp>
#include <RigCVM/TypeSystem/IType.hpp>
#include <RigCVM/TypeSystem/StructuralType.hpp>
#include <RigCVM/Scope.hpp>

namespace rigc::vm
{

class ClassType : public StructuralType
{
public:
	using Super = StructuralType;

	DynArray< DataMember > dataMembers;

	auto defaultConstructor() const -> Function*;

	auto add(DataMember mem, ParserNode const* initExpr) -> void;

	auto postInitialize(Instance& vm_) -> void override;

	auto findDataMember(StringView name) -> DataMember*;
	auto findDataMember(StringView name) const -> DataMember const*;

	auto postEvaluate(Instance& vm_) -> void;
};
}
