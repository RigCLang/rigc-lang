#include "VM/include/RigCVM/RigCVMPCH.hpp"

#include <RigCVM/Helper/String.hpp>

namespace rigc::vm
{

////////////////////////////////////////
auto replaceAll(String& s, StringView from, StringView to) -> void
{
	size_t startPos = 0;
	while((startPos = s.find(from, startPos)) != String::npos) {
		s.replace(startPos, from.length(), to);
		startPos += to.length();
	}
}


}
