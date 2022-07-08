#include "RigCVM/Environment.hpp"

namespace rigc::vm
{
fs::path getVmAppPath()
{
	std::array<char, 4 * 1024> buf;

	#ifdef PACC_SYSTEM_WINDOWS
		size_t bytes;
	#elif defined(PACC_SYSTEM_LINUX)
		ssize_t bytes;
	#endif

	// Obtain the path in `buf` and length in `bytes`
	#ifdef PACC_SYSTEM_WINDOWS
		bytes = static_cast<std::size_t>( GetModuleFileNameA(nullptr, buf.data(), static_cast<DWORD>(buf.size()) ) );
	#elif defined(PACC_SYSTEM_LINUX)
		bytes = std::min(readlink("/proc/self/exe", buf.data(), buf.size()), ssize_t(buf.size() - 1));
	#endif

	return fs::path(std::string(buf.data(), bytes));
}
}