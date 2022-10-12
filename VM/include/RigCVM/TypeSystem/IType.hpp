#pragma once

#include RIGCVM_PCH

#include <RigCVM/Helper/ExtendedVariant.hpp>

namespace rigc::vm
{
struct Instance;
struct Scope;
struct Function;

struct IType;

using AnyType		= std::any;
using InnerType		= std::shared_ptr<const IType>;
using MutInnerType	= std::shared_ptr<IType>;
using DeclType		= InnerType;
using MutDeclType	= MutInnerType;

using TemplateArgument	= ExtendedVariant<int, DeclType>; // int as a placeholder
using TemplateArguments	= std::map<String, TemplateArgument, std::less<>>;

using FunctionOverloads	= std::vector<Function*>;

/// <summary>
///		An interface for each type.
/// </summary>
struct IType : public std::enable_shared_from_this<IType>
{
	inline static auto const EmptyTemplateArguments = std::vector<TemplateArgument>{};

	virtual ~IType() = default;

	///	<summary>
	///		Complete, unique name of a type.
	///		Includes template params, etc., i.e. `Array<Char, 14>`
	///	</summary>
	virtual auto name() const -> String = 0;

	///	<summary>
	///		Symbol name of the type without template params (i.e. "Array")
	///	</summary>
	virtual auto symbolName() const -> String {
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
	virtual auto hashBasis() const -> String {
		return this->name();
	}

	///	<summary>
	///		Returns the hash of this type.
	///	</summary>
	virtual auto hash() const -> std::size_t {
		return std::hash<String>{}(this->hashBasis());
	}

	using MethodsMap = UMap<StringView, DynArray<Function*>>;
	MethodsMap methods;

	template <std::derived_from<IType> T>
	auto is() const -> bool{
		return dynamic_cast<T const*>(this) != nullptr;
	}

	template <std::derived_from<IType> T>
	auto as() const -> T const* {
		return dynamic_cast<T const*>(this);
	}

	template <std::derived_from<IType> T>
	auto as() -> T* {
		return dynamic_cast<T*>(this);
	}

	auto addMethod(StringView name_, Function* func_) -> void;

	virtual auto getTemplateArguments() const -> std::vector<TemplateArgument> const&
	{
		return EmptyTemplateArguments;
	}

	auto constructors() const -> FunctionOverloads const*;

	virtual auto postInitialize(Instance& vm_) -> void;
};

template <typename T>
struct SafeCoreTypeSize {
	static constexpr auto value = sizeof(T);
};

template <>
struct SafeCoreTypeSize<void> {
	static constexpr auto value = 0;
};

template <typename T>
auto CreateCoreType(Instance &vm_, Scope& universeScope_, StringView name_, size_t size_ = SafeCoreTypeSize<T>::value) -> IType*;

}
