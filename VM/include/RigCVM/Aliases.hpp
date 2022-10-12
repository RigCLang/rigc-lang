#pragma once

#include RIGCVM_PCH

////////////////////////////////////////
// Namespace aliases
////////////////////////////////////////
namespace rg = std::ranges;
namespace ch = std::chrono;
namespace tt = std::this_thread;
namespace fs = std::filesystem;

////////////////////////////////////////
/// Text type aliases
////////////////////////////////////////
using String		= std::string;
using StringView	= std::string_view;
using FsPath		= fs::path;

////////////////////////////////////////
/// Type aliases
////////////////////////////////////////
template <typename T>
using Opt			= std::optional<T>;
template <typename... Ts>
using Variant		= std::variant<Ts...>;
template <typename TFirst, typename TSecond>
using Pair			= std::pair<TFirst, TSecond>;
template <typename... Ts>
using Tuple			= std::tuple<Ts...>;


////////////////////////////////////////
/// Container aliases
////////////////////////////////////////
template <typename T, size_t Extent = std::dynamic_extent>
using Span			= std::span<T, Extent>;
template <typename T, size_t N>
using Array			= std::array<T, N>;
template <typename T>
using DynArray		= std::vector<T>;
template <typename TKey, typename TValue, typename TCompare = std::less<TKey>>
using Map			= std::map<TKey, TValue, TCompare>;
template <typename TElem, typename TCompare = std::less<TElem>>
using Set			= std::set<TElem, TCompare>;
template <typename TElem>
using Queue			= std::queue<TElem>;
template <typename TKey, typename TValue>
using UMap			= std::unordered_map<TKey, TValue>;



////////////////////////////////////////
/// Smart pointer aliases
////////////////////////////////////////
template <typename T>
using SharedPtr		= std::shared_ptr<T>;

template <typename T>
using UniquePtr		= std::unique_ptr<T>;

template <typename T>
using WeakPtr		= std::weak_ptr<T>;

////////////////////////////////////////
/// Threading aliases
////////////////////////////////////////
using Mutex			= std::mutex;

////////////////////////////////////////
/// Function aliases
////////////////////////////////////////
template <typename T>
using Func			= std::function<T>;
