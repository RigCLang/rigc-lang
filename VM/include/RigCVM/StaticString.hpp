#pragma once

#include RIGCVM_PCH

namespace rigc::vm
{

template <typename T, size_t MaxLength>
struct StaticString : std::array<T, MaxLength>
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
