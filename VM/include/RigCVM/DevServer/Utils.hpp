#pragma once

#include RIGCVM_PCH

#include <fmt/color.h>

namespace rigc::vm
{

auto formatStackFrameLabel(ParserNode const&) -> std::string;

template <typename T, typename... Ts>
inline void devserverLog(T&& t, Ts&&... ts)
{
	using fmt::emphasis, fmt::color;
	fmt::print(emphasis::bold | fmt::fg(color::aquamarine), "[DevServer] ");
	fmt::print(fmt::fg(color::aquamarine), std::forward<T>(t), std::forward<Ts>(ts)...);
}

}
