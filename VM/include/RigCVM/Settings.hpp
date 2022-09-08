#pragma once

#include RIGCVM_PCH

namespace rigc::vm
{

struct InstanceSettings
{
	StringView entryModuleName;

#if DEBUG // Debug-only settings
	std::chrono::milliseconds functionCallDelay{0};
	std::chrono::milliseconds warmupDuration{0};
	bool skipRootExceptionCatching = false;
	String logFilePath;
#endif
};


auto parseArgs(Span<StringView> args) -> InstanceSettings;

}
