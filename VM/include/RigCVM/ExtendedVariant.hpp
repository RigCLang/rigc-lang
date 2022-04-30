#pragma once

#include RIGCVM_PCH

namespace rigc::vm
{

template <typename... Types>
struct ExtendedVariant
	: std::variant<Types...>
{
	using BaseType = std::variant<Types...>;

	using BaseType::BaseType;

	template <typename T>
	auto is() const  -> bool{
		return std::holds_alternative<T>(*this);
	}

	template <typename T>
	T const& as() const {
		return std::get<T>(*this);
	}

	template <typename T>
	T& as() {
		return std::get<T>(*this);
	}
};

}
