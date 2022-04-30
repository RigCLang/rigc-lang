#include RIGCVM_PCH

#include <RigCVM/VM.hpp>

auto main(int argc, char* argv[]) -> int
{
	namespace pt = pegtl::parse_tree;

	if (argc < 2)
		return 0;

	// try
	// {

		rigc::vm::Instance instance;
		return instance.run(argv[1]);
	// }
	// catch (const std::exception &e)
	// {
		// std::cerr << e.what() << std::endl;
	// }
}
