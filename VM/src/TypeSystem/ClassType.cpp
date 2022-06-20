#include RIGCVM_PCH

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
auto ClassType::constructors() const
	-> FunctionOverloads const*
{
	auto ctorsIt = methods.find("construct");
	if (ctorsIt == methods.end())
		return nullptr;

	return &ctorsIt->second;
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
}
