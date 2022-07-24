#pragma once

#include RIGCVM_PCH

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

	Vec< DataMember > dataMembers;

	auto defaultConstructor() const -> Function*;

	auto add(DataMember mem, ParserNode const* initExpr) -> void;

	auto postInitialize(Instance& vm_) -> void override;

	auto postEvaluate(Instance& vm_) -> void;
};
}
