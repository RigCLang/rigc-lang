#include "VM/include/RigCVM/RigCVMPCH.hpp"

#include <RigCVM/VM.hpp>
#include <RigCVM/ErrorHandling/Exceptions.hpp>
#include <RigCVM/ErrorHandling/Formatting.hpp>
#include <RigCVM/Helper/String.hpp>
#include <RigCVM/DevServer/Instance.hpp>
#include <RigCVM/DevServer/Breakpoint.hpp>
#include <RigCVM/DevServer/Utils.hpp>
#include <RigCVM/Settings.hpp>

#include <fmt/color.h>

#ifdef DEBUG
#include <fstream>
#endif

#ifdef PACC_SYSTEM_WINDOWS
	#include <Windows.h>
#endif

namespace rvm = rigc::vm;

auto enableColors() -> void;
auto printError() -> void;

auto main(int argc, char* argv[]) -> int
{
	auto args = DynArray<StringView>();
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
				"    Please refer to https://github.com/RigCLang/rigc-lang/issues\n",

				fmt_args::error()
			);

			return -4;
		}
	};

	auto instance = rvm::Instance();
	auto settings = rvm::InstanceSettings();

	auto parsingResult = tryCatch([&]{
			settings = rvm::parseArgs(args);
			return 0;
		});
	if (parsingResult != 0)
	{
		return 1;
	}

	auto runGuarded = [&]{
		return tryCatch([&]{ return instance.run(settings); });
	};

#if DEBUG
	auto fileStream = std::make_unique<std::ofstream>(settings.logFilePath, std::ios_base::trunc);

	auto server = [&settings, &fileStream] {
		if(settings.logFilePath.empty()) {
			return std::make_unique<rvm::DevelopmentServer>(nullptr);
		}
		else {
			return std::make_unique<rvm::DevelopmentServer>(fileStream.get());
		}
	}();

	rvm::g_devServer = server.get();
	auto serverThread = std::jthread();

	instance.onInitializeDevTools = [&] {
		serverThread = std::jthread([&]{ server->run(); });

		server->onBreakpointsUpdated = [&](DynArray<rvm::Breakpoint> breakpoints) {
			instance.updateBreakpoints( std::move(breakpoints) );
		};
	};

	int returnCode;
	if (settings.skipRootExceptionCatching)
		returnCode = instance.run(settings);
	else
		returnCode = runGuarded();

	server->stop();
	return returnCode;
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

