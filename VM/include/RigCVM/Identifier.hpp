#pragma once

#include <RigCVM/RigCVMPCH.hpp>


namespace rigc::vm
{

struct Identifier
{
	enum Type
	{
		Variable,
		Function,
		FunctionTemplate,
		TypeName,

		// TODO: more to come
	};

	// TODO: decide what to do with this class.
};

}
