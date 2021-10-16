#pragma once

#include RIGCINTERPRETER_PCH

namespace rigc::vm
{

template <typename T>
struct Ref
{
	Ref() = default;

	Ref(T* ptr_)
		: ptr(ptr_)
	{}

	T* ptr;

	T&			operator*()			{ return *ptr; }
	T const&	operator*()	const	{ return *ptr; }

	bool		operator!() const	{ return ptr == nullptr; }

	explicit operator bool() const {
		return static_cast<bool>(ptr);
	}

	operator T*() const {
		return ptr;
	}

	operator T const*() const {
		return ptr;
	}
};

template <typename T>
using Ptr = T*;

struct Value
{
	using Variants = std::variant<
			bool,
			int,
			float,
			std::string,
			std::vector< Value >,
			Ptr<Value>,
			Ref<Value>
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

	bool isRef() const {
		return this->is< Ref<Value> >();
	}

	bool isPtr() const {
		return this->is< Ptr<Value> >();
	}

	Ref<Value> ref() const {
		return this->as< Ref<Value> >();
	}

	Ptr<Value> ptr() const {
		return this->as< Ptr<Value> >();
	}

	Value const& byValue() const {
		if (this->isRef())
			return *this->ref();
		
		return *this;
	}

	Value& byValue() {
		if (this->isRef())
			return *this->ref();
		
		return *this;
	}

	size_t typeIndex() const {
		return value.index();
	}

	size_t valueTypeIndex() const {
		return this->byValue().value.index();
	}
};

using OptValue = std::optional<Value>;

}