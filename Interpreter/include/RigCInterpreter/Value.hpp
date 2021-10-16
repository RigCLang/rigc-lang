#pragma once

#include RIGCINTERPRETER_PCH

namespace rigc::vm
{

struct Value
{
	using Variants = std::variant<
			int,
			float,
			std::string,
			std::vector< Value >
		>;

	Variants value;

	Value() = default;

	Value(Variants v)
		: value(std::move(v))
	{} 	

	template <typename T>
	bool is() const {
		return std::holds_alternative<T>(value);
	}

	template <typename T>
	T const& as() const {
		return std::get<T>(value);
	}

	template <typename T>
	T& as() {
		return std::get<T>(value);
	}

	template <typename T>
	T as(T alternative_) {
		if (this->is<T>())
			return std::get<T>(value);
		
		return alternative_;
	}
};

using OptValue = std::optional<Value>;

}