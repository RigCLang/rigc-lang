#include "VM/include/RigCVM/RigCVMPCH.hpp"

#include <RigCVM/VM.hpp>
#include <RigCVM/Settings.hpp>
#include <RigCVM/ErrorHandling/Exceptions.hpp>

namespace rigc::vm
{

using ProgramArgs = Span<StringView>;

struct ProgramArg {
	StringView name;
	StringView value;
};

auto findArg(ProgramArgs const& args, StringView name, bool acceptSeparate = true) -> Opt<ProgramArg>
{
	auto nameLen = name.length();
	for (size_t i = 1; i < args.size(); ++i)
	{
		if (args[i] == name) { // --program-arg [optionally a value]
			return ProgramArg(args[i], ((acceptSeparate && i + 1 < args.size()) ? args[i + 1] : StringView{}));
		}
		else if (args[i].starts_with(name) && args[i][nameLen] == '=') { // --program-name=value
			return ProgramArg(args[i].substr(0, nameLen), args[i].substr(nameLen+1));
		}
	}

	return std::nullopt;
}

template <typename T>
auto argValue(ProgramArgs const& args, StringView name) -> Opt<T> = delete;

template <typename T>
auto argValueOr(ProgramArgs const& args, StringView name, T alternative) -> T
{
	return argValue<T>(args, name).value_or(alternative);
}

template <std::integral T>
auto argValue(ProgramArgs const& args, StringView name) -> Opt<T>
{
	auto result = findArg(args, name);

	if (result)
		return static_cast<T>( std::stoll( String(result->value) ) );
	return std::nullopt;
}

template <std::floating_point T>
auto argValue(ProgramArgs const& args, StringView name) -> Opt<T>
{
	auto result = findArg(args, name);

	if (result)
		return static_cast<T>( std::stold( String(result->value) ) );
	return std::nullopt;
}

template <>
auto argValue<bool>(ProgramArgs const& args, StringView name) -> Opt<bool>
{
	auto result = findArg(args, name, false); // treat it as a flag, that is: --flag means it should return true
	return result ? Opt(result->value == "true" || result->value.empty()) : std::nullopt;
}

template <>
auto argValue<StringView>(ProgramArgs const& args, StringView name) -> Opt<StringView>
{
	auto result = findArg(args, name);
	return result ? Opt(result->value) : std::nullopt;
}

template <>
auto argValue<String>(ProgramArgs const& args, StringView name) -> Opt<String>
{
	auto result = findArg(args, name);
	return result ? Opt(String(result->value)) : std::nullopt;
}




auto parseArgs(Span<StringView> args) -> InstanceSettings
{
	// TODO:
	auto result = InstanceSettings();

	if (args.size() < 2)
	{
		auto filename = fs::path(args[0]).filename();
		throw RigCError("No entry point specified.").withHelp("Use \"{} [module name]\" to run its main function.", filename.string());
	}

	if (args[1] == "--version")
	{
		fmt::print("{} v{}\n", Instance::PrettyName, Instance::Version);
		std::exit(0);
	}

	result.entryModuleName = args[1];

#if DEBUG
	// Warmup time
	{
		constexpr auto Prefix = StringView("--warmup");

		auto warmup = argValue<int>(args, Prefix);
		if (warmup)
		{
			result.warmupDuration = std::chrono::milliseconds( *warmup );
		}
	}

	// Wait until connection
	{
		constexpr auto Prefix = StringView("--wait-for-connection");

		auto wait = argValue<bool>(args, Prefix);
		if (wait)
			result.waitForConnection = true;
	}

	// Function delay time
	{
		constexpr auto Prefix = StringView("--delay-fn");

		auto fnCallDelay = argValue<int>(args, Prefix);
		if (fnCallDelay)
		{
			result.functionCallDelay = std::chrono::milliseconds( *fnCallDelay );
		}
	}

	// skipRootExceptionCatching
	{
		constexpr auto Prefix = StringView("--skip-root-exception-catching");

		auto skip = argValue<bool>(args, Prefix);
		if (skip)
			result.skipRootExceptionCatching = true;
	}

	// Log file
	{
		constexpr auto Prefix = StringView("--log-file");

		auto logFile = argValue<StringView>(args, Prefix);
		if (logFile)
			result.logFilePath = String( *logFile );
	}
#endif

	return result;
}

}
