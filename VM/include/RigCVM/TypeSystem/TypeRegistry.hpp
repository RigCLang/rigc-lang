#pragma once

#include RIGCVM_PCH

#include <RigCVM/TypeSystem/IType.hpp>
#include <RigCVM/Symbol.hpp>

namespace rigc::vm
{

struct TypeRegistry
{
	using Hasher = std::hash<std::string_view>;

	std::unordered_map<std::size_t, Symbol<MutDeclType>> types;

	auto exists(std::string_view hash_) const -> bool;

	// Non-const
	auto find(std::size_t hash_) -> MutDeclType;
	auto find(std::string_view hashBasis_) -> MutDeclType;

	// Const
	auto find(std::size_t hash_) const -> DeclType;
	auto find(std::string_view hashBasis_) const -> DeclType;

	auto add(Symbol<MutDeclType> symbol_) -> bool;
};
}
