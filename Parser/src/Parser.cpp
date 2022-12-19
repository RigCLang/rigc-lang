#include "Parser/include/RigCParser/RigCParserPCH.hpp"

#include <RigCParser/Parser.hpp>
#include <RigCParser/Grammar.hpp>

namespace rigc
{

auto parse(p::file_input<> &in) -> ParserNodePtr
{
	namespace pt = pegtl::parse_tree;
	return pt::parse< rigc::Grammar, rigc::Selector >( in );

	// For now leave the error handling to the caller.

	// try
	// {
	// }
	// catch (const pegtl::parse_error &e)
	// {
	// 	auto const p = e.positions().front();
	// 	std::cerr << e.what() << '\n'
	// 				<< in.line_at(p) << '\n'
	// 				<< std::setw(p.column) << '^' << std::endl;
	// }
	// catch (const std::exception &e)
	// {
	// 	std::cerr << e.what() << std::endl;
	// }

	// return nullptr;
}

}
