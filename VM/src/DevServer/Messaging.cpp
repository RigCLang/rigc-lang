#include "VM/include/RigCVM/RigCVMPCH.hpp"

#include <RigCVM/DevServer/Instance.hpp>
#include <RigCVM/DevServer/Messaging.hpp>

#include <RigCVM/Helper/String.hpp>

namespace rigc::vm
{

void sendDebugMessage(String const& msg_)
{
	g_devServer->enqueueMessage(msg_);
}

auto serializeLogLevel(LogLevel level_) -> StringView
{
	switch (level_)
	{
		case LogLevel::Info: return "info";
		case LogLevel::Warning: return "warn";
		case LogLevel::Error: return "error";
	}
	return "Unknown";
}



void sendLogMessage(LogLevel level_, StringView msg_)
{
	auto escaped = String(msg_);
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
