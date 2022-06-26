#pragma once

#include RIGCVM_PCH

#include <fmt/color.h>

namespace fmt {

template <typename... Args>
void printToStream(std::ostream& stream_, fmt::format_string<Args...> fmt_string_, Args &&... args)
{
	stream_ << fmt::format(fmt_string_, std::forward<Args>(args)...);
}

template <typename... Args>
void printErr(fmt::format_string<Args...> fmt_string_, Args&&... args)
{
	printToStream(std::cerr, fmt_string_, std::forward<Args>(args)...);
}

template <typename... Args>
void printLog(fmt::format_string<Args...> fmt_string_, Args &&... args)
{
	printToStream(std::clog, fmt_string_, std::forward<Args>(args)...);
}

} // namespace fmt

namespace fmt_args
{

struct Styles {
	fmt::text_style Bold	= fmt::emphasis::bold;
	fmt::text_style Underline	= fmt::emphasis::underline;
	fmt::text_style Red		= fmt::fg(fmt::color::red);
	fmt::text_style DarkRed		= fmt::fg(fmt::color::dark_red);
	fmt::text_style Green	= fmt::fg(fmt::color::green);
	fmt::text_style Blue	= fmt::fg(fmt::color::blue);
	fmt::text_style LightBlue	= fmt::fg(fmt::color::light_blue);
	fmt::text_style Yellow	= fmt::fg(fmt::color::yellow);
};

using ArgPair = std::pair<char const*, std::string>;

inline Styles const& s() {
	static auto instance = Styles{};
	return instance;
}

#define DEFINE_FMT_ARG(funcName, argName, style, content) \
	inline auto funcName() \
	{ \
		static auto Val = fmt::format(style, content); \
		return fmt::arg(argName, Val); \
	}

DEFINE_FMT_ARG(error, 	"Error", 	s().Bold | s().Red, 	"[Error]");
DEFINE_FMT_ARG(help, 	"Help", 	s().Bold | s().Yellow, 	"[Help]");
DEFINE_FMT_ARG(details, "Details", 	s().Bold, 				"Details");

// FIXME: quickfix until I figure out how to do it sensibly
inline auto errorWithLineArgPair(std::size_t line) -> ArgPair
{
	using namespace fmt;

	auto constexpr boldRedFmt = emphasis::bold | fg(color::red);
	auto constexpr reset = "\x1b[m";

	auto const value = format(
		"{}{}{}{}{}",
		format(boldRedFmt, "[Error"),
		reset,
		" on line ",
		format(s().LightBlue, "{}", line),
		format(boldRedFmt, "]")
	);

	return {"ErrorWithLine", value};
}

#undef DEFINE_FMT_ARG

} //namespace fmt_args
