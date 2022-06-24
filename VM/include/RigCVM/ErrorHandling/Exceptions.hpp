#pragma once

#include RIGCVM_PCH

#include <stdexcept>
#include <cstdint>

struct RigcException : std::runtime_error
{
	std::string helpMessage;
	std::size_t lineNum = 0;

public:
	template <typename... Args>
	explicit RigcException(fmt::format_string<Args...> fmt_string_, Args&&... args)
		:
		std::runtime_error(fmt::format(fmt_string_, std::forward<Args>(args)...).c_str())
	{
	}

	template <typename... Args>
	auto withHelp(fmt::format_string<Args...> fmt_string_, Args&&... args) -> RigcException const&
	{
		helpMessage = fmt::format(fmt_string_, std::forward<Args>(args)...);
		return *this;
	}

	auto withLineNumber(std::size_t lineNumber) -> RigcException const&
	{
		lineNum = lineNumber;
		return *this;
	}

	auto help() const -> std::string const& { return helpMessage; }
	auto lineNumber() const -> std::size_t{ return lineNum; }
};



auto dumpException(std::exception const& exception_) -> void;

auto dumpException(RigcException const& exception_) -> void;
