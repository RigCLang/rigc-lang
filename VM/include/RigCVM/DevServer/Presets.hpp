#pragma once

#include <RigCVM/RigCVMPCH.hpp>

namespace rigc::vm::devserver_presets
{

inline constexpr auto SetBaseAddressContent = StringView(R"msg(
{{
	"type": "stack",
	"action": "setBaseAddress",
	"data": "{}"
}})msg");

inline constexpr auto SessionStartedContent = StringView(R"msg(
{{
	"type": "session",
	"action": "started"
}})msg");

inline constexpr auto SessionFinishedContent = StringView(R"msg(
{{
	"type": "session",
	"action": "finished"
}})msg");



}
