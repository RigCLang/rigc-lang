#pragma once

#include RIGCVM_PCH

#include <RigCVM/TypeSystem/Shared/DataMember.hpp>
#include <RigCVM/TypeSystem/IType.hpp>
#include <RigCVM/Scope.hpp>
#include <RigCVM/VM.hpp>

namespace rigc::vm
{
class StructuralType : public IType
{
	StringView	_name;
protected:
	std::size_t			_size = 0;
public:
	rigc::ParserNode const* declaration = nullptr;

	auto name() const -> String override
	{
		return String(_name);
	}

	auto size() const -> size_t override
	{
		return _size;
	}

	auto decay() const -> InnerType override
	{
		return const_cast<StructuralType*>(this)->shared_from_this();
	}

	auto parse(rigc::ParserNode const& node_) -> void
	{
		declaration = &node_;

		_name = findElem<rigc::Name>(node_)->string_view();
	}
};
}
