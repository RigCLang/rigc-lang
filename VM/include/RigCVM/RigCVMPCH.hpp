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

#include <fmt/format.h>
#include <fmt/args.h>

#define ASIO_STANDALONE

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

namespace rg = std::ranges;
namespace fs = std::filesystem;

using String		= std::string;
using StringView	= std::string_view;

template <typename T>
using Opt			= std::optional<T>;
template <typename... Ts>
using Variant		= std::variant<Ts...>;

template <typename T, size_t Extent = std::dynamic_extent>
using Span			= std::span<T, Extent>;
template <typename T, size_t N>
using Array			= std::array<T, N>;
template <typename T>
using DynArray		= std::vector<T>;
template <typename TKey, typename TValue>
using Map			= std::map<TKey, TValue>;
template <typename TElem>
using Set			= std::set<TElem>;
template <typename TElem>
using Queue			= std::queue<TElem>;
template <typename TKey, typename TValue>
using UMap = std::unordered_map<TKey, TValue>;
template <typename TFirst, typename TSecond>
using Pair = std::pair<TFirst, TSecond>;


template <typename T, size_t N>
inline auto viewArray(std::array<T, N>& array_, size_t offset_ = 0, std::optional<size_t> c = std::nullopt)
{
	size_t maxSize = array_.size() - offset_;
	return std::span{
		array_.data() + offset_,
		c ? (std::min(*c, maxSize)) : maxSize
	};
}
