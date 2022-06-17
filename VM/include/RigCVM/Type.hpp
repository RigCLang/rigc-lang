#pragma once

#include RIGCVM_PCH

#include <RigCVM/ExtendedVariant.hpp>
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

constexpr std::string_view OverloadedFunction = "<ovf>";
constexpr std::string_view OverloadedMethod = "<ovm>";

}

struct Operator
{
	enum Type {
		Prefix,
		Infix,
		Postfix,
	};
	std::string_view	str;
	Type				type;
};
struct TypeImpl
{
	std::map<std::string_view, Function*>	methods;
};


}
