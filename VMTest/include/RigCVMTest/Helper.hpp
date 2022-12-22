
#include <Catch2/catch_amalgamated.hpp>
#include <RigCVM/VM.hpp>

#include <string>

namespace rvm = rigc::vm;

struct TestResult
{
	int code;
	String output;
	String expected;
	String error;
	bool success;

	static auto failure() -> TestResult {
		auto result = TestResult();
		result.success = false;
		return result;
	}
};

auto freshInstance() -> std::unique_ptr<rvm::Instance>;

auto readFileToString(String const& path) -> String;

auto runTestModuleOn(
		rvm::Instance&	instance,
		StringView		sourceFile,
		String const&	expectedOutputFile,
		String const&	inputFile = "",
		bool			safe = false
	) -> TestResult;


auto runTestModule(
		StringView		sourceFile,
		String const&	expectedOutputFile,
		String const&	inputFile = "",
		bool			safe = false
	) -> TestResult;

auto runTestByName(StringView name, bool safe = true) -> TestResult;

// This shouldn't throw exceptions
auto safeRunTestByName(StringView name) -> TestResult;

// This may throw exceptions
auto unsafeRunTestByName(StringView name) -> TestResult;

