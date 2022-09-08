#include RIGCVM_PCH

#include <RigCVM/Settings.hpp>
#include <RigCVM/ErrorHandling/Exceptions.hpp>

namespace rigc::vm
{

auto parseArgs(Span<StringView> args) -> InstanceSettings
{
	// TODO:
	auto result = InstanceSettings();

	if (args.size() < 2)
	{
		throw RigCError("No entry point specified.").withHelp("Use rigcvm [module name] to run RigC script.");
	}

	result.entryModuleName = args[1];

	auto findArg = [&](StringView prefix) {
		auto it = rg::find_if(args, [&](auto a){ return a.starts_with(prefix); });
		if (it != args.end())
			return *it;
		return StringView{};
	};

#if DEBUG
	// Warmup time
	{
		constexpr auto Prefix = StringView("--warmup=");

		// Read warmup
		auto warmupArg = findArg(Prefix);
		if (!warmupArg.empty())
		{
			auto wmStr = String( warmupArg.substr(Prefix.length()) );
			result.warmupDuration = std::chrono::milliseconds( std::stoi( wmStr ) );
		}
	}

	// Warmup time
	{
		constexpr auto Prefix = StringView("--delay-fn=");

		// Read warmup
		auto warmupArg = findArg(Prefix);
		if (!warmupArg.empty())
		{
			auto wmStr = String( warmupArg.substr(Prefix.length()) );
			result.functionCallDelay = std::chrono::milliseconds( std::stoi( wmStr ) );
		}
	}

	// skipRootExceptionCatching
	{
		constexpr auto Prefix = StringView("--skip-root-exception-catching");

		// Read warmup
		auto arg = findArg(Prefix);
		if (!arg.empty())
			result.skipRootExceptionCatching = true;
	}

	// Log file
	{
		constexpr auto Prefix = StringView("--log-file=");

		auto logFileArg = findArg(Prefix);
		if (!logFileArg.empty())
			result.logFilePath = String( logFileArg.substr(Prefix.length()) );
	}
#endif

	return result;
}

}
