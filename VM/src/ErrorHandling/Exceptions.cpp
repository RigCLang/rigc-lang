#include RIGCVM_PCH

#include <RigCVM/ErrorHandling/Exceptions.hpp>
#include <RigCVM/ErrorHandling/Formatting.hpp>

auto dumpRigcExceptionWithLine(RigcException const& exception_) -> void
{
	auto const [argName, argValue] = fmt_args::errorWithLineArgPair(exception_.lineNumber());

	fmt::printErr(
		"{ErrorWithLine}. {Details}:\n\t{}\n",
		exception_.what(),
		fmt::arg(argName, argValue),
		fmt_args::details()
	);
}

auto dumpPlainException(std::exception const& exception_) -> void
{
	fmt::printErr(
		"{Error} {Details}:\n\t{}\n",
		exception_.what(),
		fmt_args::error(),
		fmt_args::details()
	);
}

auto dumpException(std::runtime_error const& exception_) -> void
{
	dumpPlainException(exception_);
}

auto dumpException(RigcException const& exception_) -> void
{
	if(exception_.lineNumber() == 0) 
		dumpPlainException(exception_);
	else 
		dumpRigcExceptionWithLine(exception_);

	if(!exception_.help().empty())
	{
		fmt::printErr("{Help}\n\t{}\n", exception_.help(), fmt_args::help());
	}
}

auto dumpException(InternalException const& exception_) -> void
{
	auto const sourceLocation = exception_.location();

	fmt::printErr(
		"{InternalError} in {File}({Line}:{Col}) [`{Function}`] {Details}:\n\t{}\n",
		exception_.what(),
		fmt_args::internalError(),
		fmt::arg("File", sourceLocation.file_name()),
		fmt::arg("Line", sourceLocation.line()),
		fmt::arg("Col", sourceLocation.column()),
		fmt::arg("Function", sourceLocation.function_name()),
		fmt_args::details()
	);
}
