#include RIGCVM_PCH

#include <RigCVM/TypeSystem/TypeRegistry.hpp>

namespace rigc::vm
{
//////////////////////////////////////////
auto TypeRegistry::exists(std::string_view hash_) const
	-> bool
{
	return types.find(Hasher{}(hash_)) != types.end();
}

//////////////////////////////////////////
auto TypeRegistry::find(std::string_view hashBasis_) const
	-> DeclType
{
	return this->find(Hasher{}(hashBasis_));
}

//////////////////////////////////////////
auto TypeRegistry::find(std::string_view hashBasis_)
	-> MutDeclType
{
	return this->find(Hasher{}(hashBasis_));
}

//////////////////////////////////////////
auto TypeRegistry::find(std::size_t hash_) const
	-> DeclType
{
	auto it = types.find(hash_);
	if (it == types.end() || (!it->second.isExported()))
		return nullptr;

	return it->second.stored;
}

//////////////////////////////////////////
auto TypeRegistry::find(std::size_t hash_)
	-> MutDeclType
{
	auto it = types.find(hash_);
	if (it == types.end() || (!it->second.isExported()))
		return nullptr;

	return it->second.stored;
}

//////////////////////////////////////////
auto TypeRegistry::add(Symbol<MutDeclType> symbol_)
	-> bool
{
	auto it = types.find(symbol_.stored->hash());
	if (it != types.end()) {
		return false;
	}
	types.insert({ symbol_.stored->hash(), std::move(symbol_) });
	return true;
}
}
