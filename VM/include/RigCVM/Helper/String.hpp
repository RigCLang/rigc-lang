#pragma once

#include RIGCVM_PCH


namespace rigc::vm
{

auto replaceAll(std::string& s, std::string_view from, std::string_view to) -> void;

}
