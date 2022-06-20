#include RIGCVM_PCH

#include <RigCVM/VM.hpp>
#include <fmt/color.h>

#ifdef PACC_SYSTEM_WINDOWS
	#include <Windows.h>
#endif

void enableColors();
void printError();

auto main(int argc, char* argv[]) -> int
{
	namespace pt = pegtl::parse_tree;

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
	catch (const std::exception &e)
	{
		printError();
		std::cerr << e.what() << '\n';
	}
	catch(...)
	{
		printError();
		std::cerr << "Unknown exception occurred.\n";
	}
#endif
}

void enableColors()
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

void printError()
{
	using color = fmt::color;
	using fmt::fg;
	using fmt::emphasis;

	fmt::print(fg(color::red) | emphasis::bold, "Error:\n");
}
