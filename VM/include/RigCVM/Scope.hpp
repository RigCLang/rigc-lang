#pragma once

#include RIGCVM_PCH

#include <RigCVM/Value.hpp>
#include <RigCVM/Type.hpp>
#include <RigCVM/Functions.hpp>
#include <RigCVM/StaticString.hpp>
#include <RigCVM/StackFrame.hpp>
#include <RigCVM/TypeSystem/TypeRegistry.hpp>
#include <RigCVM/TypeSystem/TypeConstraint.hpp>

namespace rigc::vm
{

struct Instance;

using FunctionParamTypes	= std::array<DeclType, Function::MAX_PARAMS>;
using FunctionParamTypeSpan	= std::span<DeclType>;


auto findOverload(
		FunctionCandidates		const&	funcs_,
		FunctionParamTypeSpan	paramTypes_,
		bool					method_ = false,
		Function::ReturnType	returnType_ = std::nullopt
	) -> Function const*;

auto findOverload(
		FunctionOverloads const&	overloads_,
		FunctionParamTypeSpan		paramTypes_,
		bool						method_ = false,
		Function::ReturnType		returnType_ = std::nullopt
	) -> Function const*;

using TemplateParameters = std::map<std::string, TypeConstraint, std::less<> >;

struct Scope
{
	Scope(Instance& vm_)
		: vm(&vm_)
	{
	}

	Instance* vm = nullptr;

	using Impls				= std::vector<TypeImpl*>;

	// Whether the scope belongs to a function
	// (contains parameters and local variables)
	Function const* func = nullptr;

	Scope* parent = nullptr;

	// Currently unused
	std::map<IType*, Impls*>									impls;
	std::map<std::string, IType*, std::less<> >					typeAliases;

	std::vector< std::unique_ptr<Function> >					functionStorage;
	std::map<std::string, FunctionOverloads, std::less<> >		functions;
	std::map<std::string, FunctionOverloads, std::less<> >		functionTemplates;
	std::map<std::string, FrameBasedValue, std::less<> >		variables;
	TemplateParameters											templateParams;
	TemplateArguments											templateArguments;
	TypeRegistry												types;


	/// <summary>
	/// Formats the name of an operator to get an unique name used to search for it.
	/// </summary>
	static auto formatOperatorName(std::string_view opName_, Operator::Type type_) -> StaticString<char, 512>;

	auto addType(MutDeclType type_) -> void;

	/// <summary>
	/// Returns a function converting type `from_` to type `to_`,
	/// or `nullptr` if no such conversion exist within this scope.
	/// </summary>
	auto findConversion(DeclType const& from_, DeclType const& to_) const -> Function const*;

	/// <summary>
	/// Returns all function overloads with name `funcName_`,
	/// or `nullptr` if no such function exist within this scope.
	/// </summary>
	auto findFunction(std::string_view funcName_) const -> FunctionOverloads const*;


	/// <summary>
	/// Tries to generate a function with name `funcName_` and parameter types `paramTypes_`
	/// out of the function templates registered in this scope.
	/// </summary>
	auto tryGenerateFunction(
			Instance&					vm_,
			std::string_view			funcName_,
			FunctionParamTypeSpan		paramTypes_
		) -> Function const*;

	/// <summary>
	/// Returns type with name `typeName_`,
	/// or `nullptr` if no such type exist within this scope.
	/// </summary>
	auto findType(std::string_view typeName_) const -> IType const*;

	/// <summary>
	/// Returns operator overload with name `opName_` and type `type_`,
	/// or `nullptr` if no such operator exist within this scope.
	/// </summary>
	/// <remarks>
	/// `formatOperatorName` is used within this function to get the correct name of an operator.
	/// </remarks>
	auto findOperator(std::string_view opName_, Operator::Type type_) const -> FunctionOverloads const*;

	/// <summary>
	/// Registers a type alias within this scope.
	/// </summary>
	auto registerType(Instance& vm_, std::string_view name_, IType& type_) -> IType&;

	/// <summary>
	/// Registers a function within this scope.
	/// </summary>
	auto registerFunction(Instance& vm_, std::string_view name_, Function func_) -> Function&;

	/// <summary>
	/// Registers a function template within this scope.
	/// </summary>
	auto registerFunctionTemplate(Instance& vm_, std::string_view name_, Function func_) -> Function&;

	/// <summary>
	/// Registers an operator within this scope.
	/// </summary>
	auto registerOperator(Instance& vm_, std::string_view name_, Operator::Type type_, Function func_) -> Function&;
};

std::unique_ptr<Scope> makeUniverseScope(Instance &vm_);

}
