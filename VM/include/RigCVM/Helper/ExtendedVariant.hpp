#pragma once

#include RIGCVM_PCH

namespace rigc::vm
{
template <typename... Types>
struct ExtendedVariant : Variant<Types...>
{
	using BaseType = Variant<Types...>;

	using BaseType::BaseType;

	template <typename T>
	auto is() const  -> bool{
		return std::holds_alternative<T>(*this);
	}

	template <typename T>
	auto as() const -> T const&
	{
		return std::get<T>(*this);
	}

	template <typename T>
	auto as() -> T&
	{
		return std::get<T>(*this);
	}
};
}
