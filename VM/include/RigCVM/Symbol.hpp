#pragma once

#include RIGCVM_PCH

// #include <RigCVM/Module.hpp>
#include <concepts>

namespace rigc::vm
{

struct Module;

template <typename T>
class Symbol
{
public:
	enum class Visibility {
		Exported,
		Unexported
	};

private:
	Visibility visibility;

public:

	template <std::convertible_to<T> U>
	Symbol(U&& value, Visibility visibility = Visibility::Unexported)
		: stored(std::forward<U>(value)),
		  visibility(visibility)
		{}

	auto isExported() const { return visibility == Visibility::Exported; }

	T stored;
	bool exported = false;
	Module* linkedModule;
};

}
