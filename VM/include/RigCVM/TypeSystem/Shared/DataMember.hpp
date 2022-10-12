#pragma once

#include <RigCVM/TypeSystem/IType.hpp>
#include <string>

namespace rigc::vm
{

struct DataMember
{
	String	name;
	DeclType	type;
	size_t		offset = 0;
};
}
