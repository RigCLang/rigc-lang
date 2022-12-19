#include <Catch2/catch_amalgamated.hpp>
#include <RigCVMTest/Helper.hpp>
#include <RigCVM/VM.hpp>

int main (int argc, char * argv[]) {
	return Catch::Session().run( argc, argv );
}

TEST_CASE("empty-main - empty, non returning function")
{
	auto result = unsafeRunTestByName("empty-main");

	CHECK(result.success);
	CHECK(result.output == result.expected);
}

TEST_CASE("hello-world - single usage of a print function")
{
	auto result = unsafeRunTestByName("hello-world");

	CHECK(result.success);
	CHECK(result.output == result.expected);
}

TEST_CASE("variables/create/Int32 - can create a variable of type Int32")
{
	auto result = unsafeRunTestByName("variables/create/Int32");

	CHECK(result.success);
	CHECK(result.output == result.expected);
}

TEST_CASE("extension-methods-1 - can extend the Vector2 type with a plus method")
{
	auto result = unsafeRunTestByName("extension-methods-1");

	CHECK(result.success);
	CHECK(result.output == result.expected);
}
