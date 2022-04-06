#pragma once

#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/ExtendedVariant.hpp>

namespace rigc::vm
{

struct Instance;
struct Scope;
struct Function;

using AnyType	= std::any;
using InnerType	= std::shared_ptr<struct IType>;
using DeclType	= InnerType;


/// <summary>
///		An interface for each type.
/// </summary>
struct IType
	: public std::enable_shared_from_this<IType>
{
	virtual ~IType() = default;

	///	<summary>
	///		Complete, unique name of a type.
	///		Includes template params, etc., i.e. `Array<Char, 14>`
	///	</summary>
	virtual auto name() const -> std::string = 0;

	///	<summary>
	///		Symbol name of the type without template params (i.e. "Array")
	///	</summary>
	virtual auto symbolName() const -> std::string {
		return this->name();
	}

	///	<summary>
	///		Returns the size in bytes of this type.
	///	</summary>
	virtual auto size() const -> size_t = 0;

	///	<summary>
	///		Returns the inner type (if available).
	///	</summary>
	virtual auto decay() const -> InnerType = 0;

	///	<summary>
	///		Returns <c>true</c> if this type is an array, otherwise <c>false</c>.
	///	</summary>
	virtual auto isArray() const -> bool { return false; }

	///	<summary>
	///		Returns the string that is used to calculate hash of this type.
	///	</summary>
	virtual auto hashBasis() const -> std::string {
		return this->name();
	}

	///	<summary>
	///		Returns the hash of this type.
	///	</summary>
	virtual auto hash() const -> std::size_t {
		return std::hash<std::string>{}(this->hashBasis());
	}

	using MethodsMap = UMap<std::string_view, Vec<Function*>>;
	MethodsMap methods;

	template <std::derived_from<IType> T>
	bool is() const {
		return dynamic_cast<T const*>(this) != nullptr;
	}

	template <std::derived_from<IType> T>
	T const* as() const {
		return dynamic_cast<T const*>(this);
	}

	template <std::derived_from<IType> T>
	T const* as() {
		return dynamic_cast<T*>(this);
	}

	void addMethod(std::string_view name_, Function* func_);


	virtual void postInitialize(Instance& vm_) {}
};

template <typename T>
IType* CreateCoreType(Instance &vm_, Scope& universeScope_, std::string_view name_, size_t size_ = sizeof(T));

}
