#pragma once

#include <RigCVM/RigCVMPCH.hpp>

namespace rigc::vm
{

/// @brief Helper class for using Array as a static string.
/// @tparam T
/// @tparam MaxLength
template <typename T, size_t MaxLength>
struct StaticString : Array<T, MaxLength>
{
	size_t numChars = 0;

	template <size_t OtherLen>
	constexpr
	auto operator+=(StaticString<T, OtherLen> const& other_) -> StaticString&
	{
		size_t toAdd = std::min(OtherLen, MaxLength - numChars);

		for(size_t i = 0; i < toAdd; ++i)
			(*this)[numChars + i] = other_[i];
		numChars += toAdd;

		return *this;
	}

	template <size_t OtherLen>
	constexpr
	auto operator+=(T const (&other_)[OtherLen]) -> StaticString&
	{
		size_t actualLength = OtherLen - 1;
		size_t toAdd = std::min(actualLength, MaxLength - numChars);

		for(size_t i = 0; i < toAdd; ++i)
			(*this)[numChars + i] = other_[i];
		numChars += toAdd;

		return *this;
	}

	constexpr
	auto operator+=(StringView const& other_) -> StaticString&
	{
		size_t toAdd = std::min(other_.length(), MaxLength - numChars);

		for(size_t i = 0; i < toAdd; ++i)
			(*this)[numChars + i] = other_[i];
		numChars += toAdd;

		return *this;
	}
};

}
