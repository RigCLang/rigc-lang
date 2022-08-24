#include RIGCVM_PCH

#include <RigCVM/VM.hpp>
#include <RigCVM/ErrorHandling/Exceptions.hpp>
#include <RigCVM/ErrorHandling/Formatting.hpp>
#include <RigCVM/Helper/String.hpp>
#include <RigCVM/DevServer/Instance.hpp>
#include <fmt/color.h>

#ifdef PACC_SYSTEM_WINDOWS
	#include <Windows.h>
#endif

namespace rvm = rigc::vm;

auto enableColors() -> void;
auto printError() -> void;
auto parseArgs(std::span<std::string_view> args) -> rvm::Instance::Settings;

auto main(int argc, char* argv[]) -> int
{
	auto args = std::vector< std::string_view >();
	args.reserve(argc);
	for (int i = 0; i < argc; ++i)
		args.push_back(argv[i]);

	enableColors();

	auto tryCatch = [](auto&& fn) {
		try {
			return fn();
		}
		catch(std::runtime_error const& exc)
		{
			dumpException(exc);
			return -1;
		}
		catch(RigCError const& exc)
		{
			dumpException(exc);
			return -2;
		}
		catch(...)
		{
			fmt::printErr(
				"{Error}\n"
				"    An unknown error occurred.\n"
				"    No details available\n"
				"    Please refer to https://github.com/PoetaKodu/pacc/issues\n",

				fmt_args::error()
			);

			return -4;
		}
	};

	auto instance = rvm::Instance();
	auto settings = rvm::Instance::Settings();

	tryCatch([&]{
		settings = parseArgs(args);
		return 0;
	});

	auto runGuarded = [&]{
		return tryCatch([&]{ return instance.run(settings); });
	};

#if DEBUG
	auto server = rvm::DevelopmentServer();
	rvm::g_devServer = &server;
	auto serverThread = std::jthread([&]{ server.run(); });

	if (settings.warmupDuration.count() > 0) {
		fmt::print("Warmup (time: {} ms)...\n", settings.warmupDuration.count());
		std::this_thread::sleep_for(settings.warmupDuration);
	}

	if (settings.skipRootExceptionCatching)
		return instance.run(settings);
	else
		return runGuarded();
#else
	return runGuarded();
#endif
}




auto enableColors() -> void
{
	#ifdef PACC_SYSTEM_WINDOWS
	DWORD consoleMode;
	HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (GetConsoleMode(outputHandle, &consoleMode))
	{
		SetConsoleMode(outputHandle, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
	}
	#endif
}

auto printError() -> void
{
	using color = fmt::color;
	using fmt::fg;
	using fmt::emphasis;

	fmt::print(fg(color::red) | emphasis::bold, "Error:\n");
}

auto parseArgs(std::span<std::string_view> args) -> rvm::Instance::Settings
{
	// TODO:
	auto result = rvm::Instance::Settings();

	if (args.size() < 2)
	{
		throw RigCError("No entry point specified.").withHelp("Use rigcvm [module name] to run RigC script.");
	}

	result.entryModuleName = args[1];

	auto findArg = [&](std::string_view prefix) {
		auto it = rg::find_if(args, [&](auto a){ return a.starts_with(prefix); });
		if (it != args.end())
			return *it;
		return std::string_view{};
	};

#if DEBUG
	// Warmup time
	{
		constexpr auto Prefix = std::string_view("--warmup=");

		// Read warmup
		auto warmupArg = findArg(Prefix);
		if (!warmupArg.empty())
		{
			auto wmStr = std::string( warmupArg.substr(Prefix.length()) );
			result.warmupDuration = std::chrono::milliseconds( std::stoi( wmStr ) );
		}
	}

	// Warmup time
	{
		constexpr auto Prefix = std::string_view("--delay-fn=");

		// Read warmup
		auto warmupArg = findArg(Prefix);
		if (!warmupArg.empty())
		{
			auto wmStr = std::string( warmupArg.substr(Prefix.length()) );
			result.functionCallDelay = std::chrono::milliseconds( std::stoi( wmStr ) );
		}
	}

	// skipRootExceptionCatching
	{
		constexpr auto Prefix = std::string_view("--skipRootExceptionCatching");

		// Read warmup
		auto arg = findArg(Prefix);
		if (!arg.empty())
			result.skipRootExceptionCatching = true;
	}
#endif

	return result;
}
