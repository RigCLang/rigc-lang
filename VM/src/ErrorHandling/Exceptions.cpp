#include RIGCVM_PCH

#include <RigCVM/ErrorHandling/Exceptions.hpp>
#include <RigCVM/ErrorHandling/Formatting.hpp>

auto dumpException(std::exception const& exception_) -> void
{
	fmt::printErr(
			"{Error} {Details}:\n\t{}\n",
			exception_.what(),
			fmt_args::error(),
			fmt_args::details()
	);
}

//////////////////////////////////////////////////
void dumpException(RigcException const& exception_)
{
	dumpException(static_cast<std::exception const&>(exception_));

	if(exception_.help().empty())	 return;

	fmt::printErr("{Help}\n\t{}\n", exception_.help(), fmt_args::help());
}
