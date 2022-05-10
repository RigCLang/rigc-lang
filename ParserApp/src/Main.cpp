#include <RigCParser/Grammar.hpp>
#include <RigCParser/Parser.hpp>

#include <RigCParserApp/pretty_printer.hpp>
#include <RigCParserApp/helpers.hpp>
#include <RigCParserApp/colors.hpp>
#include <RigCParserApp/cli.hpp>

#include <filesystem>

auto main(int argc, char** argv) -> int {
	std::ios_base::sync_with_stdio(false);
	std::cout.tie(nullptr);

	namespace fs = std::filesystem;

	const auto app_name = fs::path(argv[0]).filename().string();
	if(argc < 2) {
		cli::print_help(app_name);
		return 0;
	}

	try {
		auto const[help_invoked, indent, filename] = cli::parse(argc, argv);

		if(help_invoked) {
			cli::print_help(app_name);
			return 0;
		}

		if(not fs::exists(filename)) {
			println(F::Red, "[Error] ", F::Reset, "File ", F::Blue, "\"", filename, "\"", F::Reset, " not found!");
			return -1;
		}

		pegtl::file_input in(filename);

		auto root = rigc::parse( in );

		if(root) {
			pretty_printer::pretty_print_nodes(*root, 0, indent);
		}

	} catch(std::exception const& ex) {
		println(ex.what());
	}

}
