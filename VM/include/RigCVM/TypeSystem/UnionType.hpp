#pragma once

#include <RigCVM/RigCVMPCH.hpp>

#include <RigCVM/TypeSystem/Shared/DataMember.hpp>
#include <RigCVM/TypeSystem/StructuralType.hpp>
#include <RigCVM/Scope.hpp>
#include <RigCVM/VM.hpp>

namespace rigc::vm
{
class UnionType

	:
	public StructuralType
{
	public:
	auto add(DataMember mem, ParserNode const* initExpr) -> void {
		// TODO: implement this
		_size = rg::max(mem.type->size(), _size);
	}
};
}
