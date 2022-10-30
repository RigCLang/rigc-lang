#pragma once

#include <RigCVM/RigCVMPCH.hpp>

#include <RigCVM/TypeSystem/IType.hpp>

namespace rigc::vm
{

struct TypeRegistry
{
	std::unordered_map<std::size_t, MutDeclType> types;

	using Hasher = std::hash<StringView>;

	auto exists(StringView hash_) const -> bool;

	// Non-const
	auto find(std::size_t hash_) -> MutDeclType;
	auto find(StringView hashBasis_) -> MutDeclType;

	// Const
	auto find(std::size_t hash_) const -> DeclType;
	auto find(StringView hashBasis_) const -> DeclType;

	auto add(MutDeclType type_) -> bool;
};
}
