#pragma once

#include <RigCVM/RigCVMPCH.hpp>

namespace rigc::vm
{

struct InstanceSettings
{
	StringView entryModuleName;

	struct CustomStreams {
		std::ostream* out = &std::cout;
		std::ostream* err = &std::cerr;
		std::ostream* log = &std::clog;
		std::istream* in = &std::cin;
	} streams;

#if DEBUG // Debug-only settings
	std::chrono::milliseconds functionCallDelay{0};
	std::chrono::milliseconds warmupDuration{0};
	bool skipRootExceptionCatching = false;
	String logFilePath;
	bool waitForConnection = false;
#endif
};


auto parseArgs(Span<StringView> args) -> InstanceSettings;

}
