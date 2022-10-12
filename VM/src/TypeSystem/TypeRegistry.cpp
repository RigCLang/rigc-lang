#include RIGCVM_PCH

#include <RigCVM/TypeSystem/TypeRegistry.hpp>

namespace rigc::vm
{
//////////////////////////////////////////
auto TypeRegistry::exists(StringView hash_) const
	-> bool
{
	return types.find(Hasher{}(hash_)) != types.end();
}

//////////////////////////////////////////
auto TypeRegistry::find(StringView hashBasis_) const
	-> DeclType
{
	return this->find(Hasher{}(hashBasis_));
}

//////////////////////////////////////////
auto TypeRegistry::find(StringView hashBasis_)
	-> MutDeclType
{
	return this->find(Hasher{}(hashBasis_));
}

//////////////////////////////////////////
auto TypeRegistry::find(std::size_t hash_) const
	-> DeclType
{
	auto it = types.find(hash_);
	if (it == types.end())
		return nullptr;
	return it->second;
}

//////////////////////////////////////////
auto TypeRegistry::find(std::size_t hash_)
	-> MutDeclType
{
	auto it = types.find(hash_);
	if (it == types.end())
		return nullptr;
	return it->second;
}

//////////////////////////////////////////
auto TypeRegistry::add(MutDeclType type_)
	-> bool
{
	auto it = types.find(type_->hash());
	if (it != types.end()) {
		return false;
	}
	types.insert({ type_->hash(), type_ });
	return true;
}
}
