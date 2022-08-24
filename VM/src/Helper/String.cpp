#include RIGCVM_PCH

#include <RigCVM/Helper/String.hpp>

namespace rigc::vm
{

////////////////////////////////////////
auto replaceAll(std::string& s, std::string_view from, std::string_view to) -> void
{
	size_t startPos = 0;
	while((startPos = s.find(from, startPos)) != std::string::npos) {
		s.replace(startPos, from.length(), to);
		startPos += to.length();
	}
}


}
