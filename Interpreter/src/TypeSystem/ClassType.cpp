#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/TypeSystem/ClassType.hpp>
#include <RigCInterpreter/VM.hpp>

namespace rigc::vm
{

///////////////////////////////////////////
void ClassType::parse(rigc::ParserNode const& node_)
{
	declaration = &node_;

	_name = findElem<rigc::Name>(node_)->string_view();
	// fmt::print("Class name: {}, size: {}\n", _name, _size);
}

///////////////////////////////////////////
auto ClassType::defaultConstructor() const
	 -> Function*
{
	auto ctorsIt = methods.find("construct");
	if (ctorsIt == methods.end())
		return nullptr;

	auto const& ctors = ctorsIt->second;
	auto def = rg::find(ctors, 1, &Function::paramCount);
	if (def == ctors.end())
		return nullptr;

	return *def;
}

}
