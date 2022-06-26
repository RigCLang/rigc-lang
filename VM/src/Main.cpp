#include RIGCVM_PCH

#include <RigCVM/VM.hpp>
#include <RigCVM/ErrorHandling/Exceptions.hpp>
#include <RigCVM/ErrorHandling/Formatting.hpp>
#include <fmt/color.h>

#ifdef PACC_SYSTEM_WINDOWS
	#include <Windows.h>
#endif

auto enableColors() -> void;
auto printError() -> void;

auto main(int argc, char* argv[]) -> int
{
	enableColors();

	if (argc < 2)
	{
		using color = fmt::color;
		using fmt::fg;
		using fmt::emphasis;

		printError();
		fmt::print(fg(color::yellow) | emphasis::bold, "Hint:\n");
		fmt::print("    run the following command\n");
		fmt::print(fg(color::gray), "    {} <filename>\n", argv[0]);
		return 0;
	}

#if DEBUG
		rigc::vm::Instance instance;
		return instance.run(argv[1]);
#else
	try
	{
		rigc::vm::Instance instance;
		return instance.run(argv[1]);
	}
	catch(std::runtime_error const& exc)
	{
		dumpException(exc);
		return -1;
	}
	catch(RigcException const& exc)
	{
		dumpException(exc);
		return -2;
	}
	catch(InternalException const& exc)
	{
		dumpException(exc);
		return -3;
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
