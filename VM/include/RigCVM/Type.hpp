#pragma once

#include RIGCVM_PCH

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

namespace BuiltinTypes
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
}
