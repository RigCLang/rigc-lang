#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/VM.hpp>

int main(int argc, char* argv[])
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
