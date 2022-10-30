#include "VM/include/RigCVM/RigCVMPCH.hpp"

#include <RigCVM/TypeSystem/ClassType.hpp>
#include <RigCVM/VM.hpp>

namespace rigc::vm
{
// ///////////////////////////////////////////
// void ClassType::parse(rigc::ParserNode const& node_)
// {
// 	declaration = &node_;

// 	_name = findElem<rigc::Name>(node_)->string_view();
// }

auto ClassType::add(DataMember mem, [[maybe_unused]] ParserNode const* initExpr) -> void
{
	// TODO: actually do something with the initExpr
	mem.offset = _size;
	_size += mem.type->size();

	dataMembers.emplace_back(std::move(mem));
}

///////////////////////////////////////////
auto ClassType::defaultConstructor() const
	 -> Function*
{
	auto ctors = constructors();
	if (!ctors)
		return nullptr;

	auto def = rg::find(*ctors, 1, &Function::paramCount);
	if (def == ctors->end())
		return nullptr;

	return *def;
}

auto ClassType::findDataMember(StringView name) -> DataMember*
{
	auto it = rg::find(dataMembers, name, &DataMember::name);
	if (it == dataMembers.end())
		return nullptr;
	return &*it;
}

auto ClassType::findDataMember(StringView name) const -> DataMember const*
{
	auto it = rg::find(dataMembers, name, &DataMember::name);
	if (it == dataMembers.end())
		return nullptr;
	return &*it;
}

///////////////////////////////////////////
auto ClassType::postInitialize(Instance& vm_) -> void
{
	// None
}

///////////////////////////////////////////
auto ClassType::postEvaluate(Instance& vm_) -> void
{
	Super::postInitialize(vm_);
}

}
