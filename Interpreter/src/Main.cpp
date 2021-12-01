#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/VM.hpp>

int main(int argc, char* argv[])
{
	namespace pt = pegtl::parse_tree;

	if (argc < 2)
		return 0;

	// try
	// {
		pegtl::file_input in(argv[1]);

		auto root = rigc::parse( in );
		if (root) {
			return rigc::vm::runProgram(root);
		}
	// }
	// catch (const std::exception &e)
	// {
		// std::cerr << e.what() << std::endl;
	// }
}

