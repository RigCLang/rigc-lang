#pragma once

#include RIGCVM_PCH

#include <RigCVM/TypeSystem/IType.hpp>
#include <RigCVM/Scope.hpp>

namespace rigc::vm
{

struct DataMember {
	std::string	name;
	DeclType	type;
	size_t		offset = 0;
};

class ClassType

	:
	public IType
{
	std::string_view	_name;
	std::size_t			_size = 0;
public:
	rigc::ParserNode const* declaration = nullptr;


	auto name() const -> std::string override
	{
		return std::string(_name);
	}

	auto size() const -> size_t override {
		return _size;
	}

	auto decay() const -> InnerType override {
		return const_cast<ClassType*>(this)->shared_from_this();
	}

	inline auto add(DataMember mem) -> void
	{
		mem.offset = _size;
		_size += mem.type->size();
		dataMembers.emplace_back(std::move(mem));
	}

	Vec< DataMember > dataMembers;

	auto defaultConstructor() const -> Function*;
	auto constructors() const -> FunctionOverloads const*;

	auto parse(rigc::ParserNode const& node_) -> void;
};

}
