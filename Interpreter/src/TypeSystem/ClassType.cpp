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

}
