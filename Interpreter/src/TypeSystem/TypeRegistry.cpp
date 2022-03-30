#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/TypeSystem/TypeRegistry.hpp>


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
auto TypeRegistry::find(std::size_t hash_) const
	-> DeclType
{
	auto it = types.find(hash_);
	if (it == types.end())
		return nullptr;
	return it->second;
}

//////////////////////////////////////////
auto TypeRegistry::add(DeclType type_)
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
