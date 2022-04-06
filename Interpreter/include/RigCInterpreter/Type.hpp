#pragma once

#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/ExtendedVariant.hpp>
#include <RigCInterpreter/TypeSystem/IType.hpp>
#include <RigCInterpreter/TypeSystem/CoreType.hpp>
#include <RigCInterpreter/TypeSystem/TypeRegistry.hpp>


namespace rigc::vm
{

// // Forward declarations:
struct Function;
struct ArrayType;
struct WrapperType;
struct RefType;

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
