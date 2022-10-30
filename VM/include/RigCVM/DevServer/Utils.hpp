#pragma once

#include <RigCVM/RigCVMPCH.hpp>

#include <fmt/color.h>

namespace rigc::vm
{

auto formatStackFrameLabel(ParserNode const&) -> String;

template <typename T, typename... Ts>
inline void devserverLog(T&& t, Ts&&... ts)
{
	using fmt::emphasis, fmt::color;
	fmt::print(emphasis::bold | fmt::fg(color::aquamarine), "[DevServer] ");
	fmt::print(fmt::fg(color::aquamarine), std::forward<T>(t), std::forward<Ts>(ts)...);
}

}
