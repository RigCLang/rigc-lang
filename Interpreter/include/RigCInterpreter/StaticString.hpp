#pragma once

#include RIGCINTERPRETER_PCH

namespace rigc::vm
{

template <typename T, size_t MaxLength>
struct StaticString
	 : std::array<T, MaxLength>
{
	size_t numChars = 0;

	template <size_t OtherLen>
	constexpr
	StaticString& operator+=(StaticString<T, OtherLen> const& other_)
	{
		size_t toAdd = std::min(OtherLen, MaxLength - numChars);

		for(size_t i = 0; i < toAdd; ++i)
			(*this)[numChars + i] = other_[i];
		numChars += toAdd;

		return *this;
	}

	template <size_t OtherLen>
	constexpr
	StaticString& operator+=(T const (&other_)[OtherLen])
	{
		size_t actualLength = OtherLen - 1;
		size_t toAdd = std::min(actualLength, MaxLength - numChars);

		for(size_t i = 0; i < toAdd; ++i)
			(*this)[numChars + i] = other_[i];
		numChars += toAdd;

		return *this;
	}

	constexpr
	StaticString& operator+=(std::string_view const& other_)
	{
		size_t toAdd = std::min(other_.length(), MaxLength - numChars);

		for(size_t i = 0; i < toAdd; ++i)
			(*this)[numChars + i] = other_[i];
		numChars += toAdd;

		return *this;
	}
};

}
