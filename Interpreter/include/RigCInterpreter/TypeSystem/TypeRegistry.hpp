#pragma once

#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/TypeSystem/IType.hpp>

namespace rigc::vm
{

struct TypeRegistry
{
	std::unordered_map<std::size_t, DeclType> types;

	using Hasher = std::hash<std::string_view>;

	auto exists(std::string_view hash_) const -> bool;
	auto find(std::size_t hash_) const -> DeclType;
	auto find(std::string_view hashBasis_) const -> DeclType;
	auto add(DeclType type_) -> bool;
};


}
