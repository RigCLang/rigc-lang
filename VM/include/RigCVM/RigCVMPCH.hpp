#pragma once

#include <RigCParser/Grammar.hpp>
#include <RigCParser/Parser.hpp>

#include <span>
#include <array>
#include <vector>
#include <memory>
#include <algorithm>
#include <map>
#include <set>
#include <unordered_map>
#include <optional>
#include <variant>
#include <any>
#include <ranges>
#include <stack>
#include <string>
#include <string_view>
#include <cstring>
#include <functional>
#include <cassert>
#include <filesystem>
#include <chrono>

#include <fmt/format.h>
#include <fmt/args.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

#define ASIO_STANDALONE

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <RigCVM/Aliases.hpp>

template <typename T, size_t N>
inline auto viewArray(std::array<T, N>& array_, size_t offset_ = 0, std::optional<size_t> c = std::nullopt)
{
	size_t maxSize = array_.size() - offset_;
	return std::span{
		array_.data() + offset_,
		c ? (std::min(*c, maxSize)) : maxSize
	};
}
