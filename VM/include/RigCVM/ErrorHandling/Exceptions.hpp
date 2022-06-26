#pragma once

#include RIGCVM_PCH

#include <source_location>
#include <stdexcept>
#include <cstdint>

struct RigcException : std::exception
{
	std::string basicMessage;

	std::string helpMessage;
	std::size_t lineNum = 0;

public:
	template <typename... Args>
	explicit RigcException(fmt::format_string<Args...> fmt_string_, Args&&... args)
		:
		basicMessage(fmt::format(fmt_string_, std::forward<Args>(args)...))
	{
	}

	auto what() const noexcept -> const char* override { 
		return basicMessage.c_str(); 
	}

	template <typename... Args>
	auto withHelp(fmt::format_string<Args...> fmt_string_, Args&&... args) -> RigcException&
	{
		helpMessage = fmt::format(fmt_string_, std::forward<Args>(args)...);
		return *this;
	}

	auto withLine(std::size_t lineNumber) -> RigcException&
	{
		lineNum = lineNumber;
		return *this;
	}

	auto help() const -> std::string const& { return helpMessage; }
	auto lineNumber() const -> std::size_t{ return lineNum; }
};


auto dumpException(std::runtime_error const& exception_) -> void;
auto dumpException(RigcException const& exception_) -> void;
