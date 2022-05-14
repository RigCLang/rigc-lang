#include RIGCVM_PCH

#include <RigCVM/TypeSystem/EnumType.hpp>

namespace rigc::vm
{

auto EnumType::add(DataMember mem, OptValue const& val) -> void {
	// TODO: actually do something with the initExpr
	_size = mem.type->size();

	if(!val) {
		throw std::runtime_error("Automatic enum indexing not implemented yet.");
	}

	auto const[it, insertionHappened] = fields.try_emplace( mem.name, *val );

	if(!insertionHappened)
		throw std::runtime_error(fmt::format("Member {} already present in the enum.\n", mem.name));
}

}
