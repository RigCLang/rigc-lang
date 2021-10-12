#include RIGC_PCH

#include <RigC/Grammar.hpp>

int main(int argc, char *argv[])
{
	if (argc < 2)
		return 0;

	// The surrounding try/catch for normal exceptions.
	// These might occur if a file can not be opened, etc.
	try
	{
		tao::pegtl::file_input in(argv[1]);

		// The inner try/catch block, see below...
		try
		{

			// The actual parser, tracer, parse tree, ...
			auto root = pegtl::parse_tree::parse< rigc::Grammar, rigc::Selector >( in );
			if (root) {
				pegtl::parse_tree::print_dot( std::cout, *root );
			}
		}
		catch (const pegtl::parse_error &e)
		{

			// This catch block needs access to the input
			const auto p = e.positions().front();
			std::cerr << e.what() << '\n'
					  << in.line_at(p) << '\n'
					  << std::setw(p.column) << '^' << std::endl;
		}
	}
	catch (const std::exception &e)
	{

		// Generic catch block for other exceptions
		std::cerr << e.what() << std::endl;
	}
}