#pragma once

#include <RigCVM/TypeSystem/IType.hpp>
#include <string>

namespace rigc::vm
{

struct DataMember {
	std::string	name;
	DeclType	type;
	size_t		offset = 0;
};

}
