#pragma once

#include <RigCVM/RigCVMPCH.hpp>

#include <RigCVM/Helper/ExtendedVariant.hpp>
#include <RigCVM/TypeSystem/IType.hpp>
#include <RigCVM/TypeSystem/CoreType.hpp>
#include <RigCVM/TypeSystem/TypeRegistry.hpp>

namespace rigc::vm
{
// // Forward declarations:
struct Function;
struct ArrayType;
struct TemplateType;
struct RefType;

namespace builtin_types
{
constexpr auto OverloadedFunction = StringView("<ovf>");
constexpr auto OverloadedMethod = StringView("<ovm>");
}

struct Operator
{
	enum Type {
		Prefix,
		Infix,
		Postfix,
	};
	StringView			str;
	Type				type;
};
struct TypeImpl
{
	Map<StringView, Function*>	methods;
};

struct BuiltinTypes
{
	struct Accessor {
		IType* raw = nullptr;
		auto shared()		{ return raw->shared_from_this(); }
		auto shared() const	{ return raw->shared_from_this(); }
		auto weak() const	{ return raw->weak_from_this(); }
		auto weak()			{ return raw->weak_from_this(); }

		operator IType*() const { return raw; }
	};

	Accessor Void;
	Accessor Null;
	Accessor Int16;
	Accessor Int32;
	Accessor Int64;
	Accessor Uint16;
	Accessor Uint32;
	Accessor Uint64;
	Accessor Float32;
	Accessor Float64;
	Accessor Char;
	Accessor Char16;
	Accessor Char32;
	Accessor Bool;
};

}
