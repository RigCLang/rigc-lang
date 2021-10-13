#include <RigCParser/Grammar.hpp>
#include <RigCParser/Parser.hpp>

int main(int argc, char *argv[])
{
	namespace pt = pegtl::parse_tree;

	if (argc < 2)
		return 0;

	try
	{
		pegtl::file_input in(argv[1]);

		auto root = rigc::parse( in );
		if (root) {
			pt::print_dot( std::cout, *root );
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
}