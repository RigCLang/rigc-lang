#pragma once

#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/TypeSystem/IType.hpp>
#include <RigCInterpreter/Scope.hpp>

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


	std::string name() const override
	{
		return std::string(_name);
	}

	auto size() const -> size_t {
		return _size;
	}

	auto decay() const -> InnerType {
		return const_cast<ClassType*>(this)->shared_from_this();
	}



	inline void add(DataMember mem)
	{
		mem.offset = _size;
		_size += mem.type->size();
		dataMembers.emplace_back(std::move(mem));
	}

	Vec< DataMember > dataMembers;

	auto defaultConstructor() const -> Function*;

	void parse(rigc::ParserNode const& node_);
};

}
