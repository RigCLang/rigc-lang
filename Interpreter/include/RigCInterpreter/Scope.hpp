#pragma once

#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Value.hpp>
#include <RigCInterpreter/Type.hpp>
#include <RigCInterpreter/Functions.hpp>
#include <RigCInterpreter/InlineString.hpp>
#include <RigCInterpreter/StackFrame.hpp>
#include <RigCInterpreter/TypeSystem/TypeRegistry.hpp>

namespace rigc::vm
{

struct Instance;

using FunctionParamTypes	= std::array<DeclType, Function::MAX_PARAMS>;


Function const* findOverload(
		FunctionOverloads const&	funcs_,
		FunctionParamTypes const&	paramTypes_,
		size_t						numArgs_,
		Function::ReturnType		returnType_ = std::nullopt
	);

struct Scope
{

	using Impls				= std::vector<TypeImpl*>;

	// Whether the scope belongs to a function
	// (contains parameters and local variables)
	bool func = false;

	Scope* parent = nullptr;

	std::map<IType*, Impls*>									impls;
	std::vector< std::unique_ptr<Function> >					functionStorage;
	std::map<std::string, FunctionOverloads, std::less<> >		functions;
	std::map<std::string, FrameBasedValue, std::less<> >		variables;
	std::map<std::string, IType*, std::less<> >					typeAliases;
	TypeRegistry												types;

	static StaticString<char, 512> formatOperatorName(std::string_view opName_, Operator::Type type_);

	Function const* findConversion(DeclType const& from_, DeclType const& to_) const;

	FunctionOverloads const* findFunction(std::string_view funcName_) const;

	IType const* findType(std::string_view typeName_) const;

	FunctionOverloads const* findOperator(std::string_view opName_, Operator::Type type_) const;

	IType& registerType(Instance& vm_, std::string_view name_, IType& type_);
	Function& registerFunction(Instance& vm_, std::string_view name_, Function func_);
	Function& registerOperator(Instance& vm_, std::string_view name_, Operator::Type type_, Function func_);
};

std::unique_ptr<Scope> makeUniverseScope(Instance &vm_);

}
