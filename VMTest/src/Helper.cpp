#include <RigCVMTest/Helper.hpp>
#include <fstream>
#include <sstream>

auto freshInstance() -> std::unique_ptr<rvm::Instance> {
	return std::make_unique<rvm::Instance>();
}

auto readFileToString(String const& path) -> String {
	auto file = std::ifstream(path);

	if (!file.is_open()) {
		return String();
	}

	auto str = String();
	std::array<char, 16 * 1024> buffer;
	while (file.read(buffer.data(), buffer.size())) {
		str.append(buffer.data(), buffer.size());
	}
	str.append(buffer.data(), file.gcount());
	return str;
}

auto runTestModuleOn(
		rvm::Instance& instance,
		StringView sourceFile,
		String const& expectedOutputFile,
		String const& inputFile,
		bool safe
	) -> TestResult
{
	auto settings = rvm::InstanceSettings();

	auto std_out = std::ostringstream();
	auto std_err = std::ostringstream();
	auto std_log = std::ostringstream();

	auto input = readFileToString(inputFile);

	auto std_in = std::istringstream(input);

	settings.streams.in = &std_in;
	settings.streams.out = &std_out;
	settings.streams.err = &std_err;
	settings.streams.log = &std_log;

	auto sourceFilePath = String("tests/");
	sourceFilePath += sourceFile;

	auto resultFilePath = String("tests/");
	resultFilePath += expectedOutputFile;

	settings.entryModuleName = sourceFilePath;

	auto result = TestResult();
	result.success = true;

	if (safe) {
		try {
			result.code = instance.run(settings);
		}
		catch(std::exception& err) {
			// fmt::print("TEST CASE HANDLED EXCEPTION: {}\n", err.what());
			result.success = false;
		}
		catch(...) {
			// fmt::print("TEST CASE HANDLED UNKNOWN EXCEPTION\n");
			result.success = false;
		}
	}
	else {
		result.code = instance.run(settings);
	}

	result.output = std_out.str();
	result.expected = readFileToString(resultFilePath);
	result.error = std_err.str();
	return result;
}

auto runTestModule(
		StringView sourceFile,
		String const& expectedOutputFile,
		String const& inputFile,
		bool safe
	) -> TestResult
{
	auto vm = freshInstance();
	return runTestModuleOn(*vm, sourceFile, expectedOutputFile, inputFile, safe);
}

auto runTestByName(StringView name, bool safe)
	-> TestResult
{
	return runTestModule(
		fmt::format("{}/main.rigc", name),
		fmt::format("{}/expected-output.txt", name),
		fmt::format("{}/input.txt", name)
	);
}

auto safeRunTestByName(StringView name)
	-> TestResult
{
	return runTestByName(name, true);
}

auto unsafeRunTestByName(StringView name)
	-> TestResult
{
	return runTestByName(name, false);
}
