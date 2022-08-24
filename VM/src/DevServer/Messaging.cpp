#include RIGCVM_PCH

#include <RigCVM/DevServer/Instance.hpp>
#include <RigCVM/DevServer/Messaging.hpp>

#include <RigCVM/Helper/String.hpp>

namespace rigc::vm
{

void sendDebugMessage(std::string const& msg_)
{
	g_devServer->enqueueMessage(msg_);
}

auto serializeLogLevel(LogLevel level_) -> std::string_view
{
	switch (level_)
	{
		case LogLevel::Info: return "info";
		case LogLevel::Warning: return "warn";
		case LogLevel::Error: return "error";
	}
	return "Unknown";
}



void sendLogMessage(LogLevel level_, std::string_view msg_)
{
	auto escaped = std::string(msg_);
	rigc::vm::replaceAll(escaped, "\"", "\\\"");

	sendDebugMessage(fmt::format(
R"msg(
{{
	"type": "log",
	"data": {{
		"type": "{}",
		"message": "{}"
	}}
}}
)msg", serializeLogLevel(level_), escaped));
}



}
