#include RIGCVM_PCH

#include <RigCVM/ErrorHandling/Exceptions.hpp>
#include <RigCVM/ErrorHandling/Formatting.hpp>

auto dumpExceptionWithLine(RigcException const& exception_) -> void 
{
	auto const [argName, argValue] = fmt_args::errorWithLineArgPair(exception_.lineNumber());

	fmt::printErr(
		"{ErrorWithLine}. {Details}:\n\t{}\n",
		exception_.what(),
		fmt::arg(argName, argValue),
		fmt_args::details()
	);
}

auto dumpExceptionWithoutLine(RigcException const& exception_) -> void 
{
	fmt::printErr(
		"{Error} {Details}:\n\t{}\n",
		exception_.what(),
		fmt_args::error(),
		fmt_args::details()
	);
}

auto dumpException(RigcException const& exception_) -> void
{
	if(exception_.lineNumber() == 0) 
		dumpExceptionWithoutLine(exception_);
	else 
		dumpExceptionWithLine(exception_);

	if(!exception_.help().empty())
	{
		fmt::printErr("{Help}\n\t{}\n", exception_.help(), fmt_args::help());
	}
}
