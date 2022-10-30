#include "Parser/include/RigCParser/RigCParserPCH.hpp"

#include <RigCParser/Parser.hpp>
#include <RigCParser/Grammar.hpp>

namespace rigc
{

auto parse(p::file_input<> &in) -> ParserNodePtr
{
	namespace pt = pegtl::parse_tree;

	try
	{
		// The actual parser, tracer, parse tree, ...
		return pt::parse< rigc::Grammar, rigc::Selector >( in );
	}
	catch (const pegtl::parse_error &e)
	{
		// This catch block needs access to the input
		auto const p = e.positions().front();
		std::cerr << e.what() << '\n'
					<< in.line_at(p) << '\n'
					<< std::setw(p.column) << '^' << std::endl;
	}
	catch (const std::exception &e)
	{
		// Generic catch block for other exceptions
		std::cerr << e.what() << std::endl;
	}

	return nullptr;
}

}
