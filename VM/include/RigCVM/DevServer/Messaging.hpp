#pragma once

#include RIGCVM_PCH

namespace rigc::vm
{

enum class LogLevel
{
	Info,
	Warning,
	Error,
};

auto serializeLogLevel(LogLevel level_) -> std::string_view;

void sendDebugMessage(std::string const& msg_);

void sendLogMessage(LogLevel level_, std::string_view msg_);

template <typename Arg, typename... Args>
void sendLogMessage(LogLevel level_, std::string_view msg_, Arg&& arg_, Args&&... args_)
{
	sendLogMessage(level_, fmt::format(msg_, std::forward<Arg>(arg_), std::forward<Args>(args_)...));
}

}
