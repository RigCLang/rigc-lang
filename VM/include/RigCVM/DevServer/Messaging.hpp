#pragma once

#include <RigCVM/RigCVMPCH.hpp>

namespace rigc::vm
{

enum class LogLevel
{
	Info,
	Warning,
	Error,
};

auto serializeLogLevel(LogLevel level_) -> StringView;

void sendDebugMessage(String const& msg_);

void sendLogMessage(LogLevel level_, StringView msg_);

template <typename Arg, typename... Args>
void sendLogMessage(LogLevel level_, StringView msg_, Arg&& arg_, Args&&... args_)
{
	sendLogMessage(level_, fmt::format(fmt::runtime(msg_), std::forward<Arg>(arg_), std::forward<Args>(args_)...));
}

}
