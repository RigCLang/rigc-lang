#pragma once

#include <RigCParser/Grammar.hpp>
#include <RigCParser/Parser.hpp>

#include <array>
#include <vector>
#include <memory>
#include <algorithm>
#include <map>
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

#include <fmt/format.h>
#include <fmt/args.h>

namespace rg = std::ranges;

template <typename T>
using Opt = std::optional<T>;

template <typename T>
using Vec = std::vector<T>;

template <typename TKey, typename TValue>
using Map = std::map<TKey, TValue>;

template <typename TKey, typename TValue>
using UMap = std::unordered_map<TKey, TValue>;

// pair
template <typename TFirst, typename TSecond>
using Pair = std::pair<TFirst, TSecond>;
