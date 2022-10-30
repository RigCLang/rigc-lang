#pragma once

#include <RigCVM/RigCVMPCH.hpp>

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

using FunctionParamTypes	= Array<DeclType, Function::MAX_PARAMS>;
using FunctionParamTypeSpan	= Span<DeclType>;

auto findOverload(
		FunctionCandidates		const&	funcs_,
		FunctionParamTypeSpan	paramTypes_,
		bool					method_ = false,
		Function::ReturnType	returnType_ = nullptr
	) -> Function const*;

auto findOverload(
		FunctionOverloads const&	overloads_,
		FunctionParamTypeSpan		paramTypes_,
		bool						method_ = false,
		Function::ReturnType		returnType_ = nullptr
	) -> Function const*;

using TemplateParameters = Map<String, TypeConstraint, std::less<> >;

template <typename T>
struct ScopeTraceResult {
	Scope const*	scope;
	T				value;
};

struct Scope
{
	Scope(Instance& vm_)
		: vm(&vm_)
	{
	}

	Instance* vm = nullptr;

	using Impls	= DynArray<TypeImpl*>;

	// Whether the scope belongs to a function
	// (contains parameters and local variables)
	Function const* func = nullptr;

	Scope* parent = nullptr;
	void* addr = nullptr;

#if DEBUG
	String name = "";
#endif

	// Currently unused
	Map<IType*, Impls*>								impls;
	Map<String, IType*, std::less<> >				typeAliases;

	DynArray< UniquePtr<Function> >		functionStorage;
	Map<String, FunctionOverloads, std::less<> >	functions;
	Map<String, FunctionOverloads, std::less<> >	functionTemplates;
	Map<String, FrameBasedValue, std::less<> >		variables;
	TemplateParameters								templateParams;
	TemplateArguments								templateArguments;
	TypeRegistry									types;

	/// <summary>
	/// Formats the name of an operator to get an unique name used to search for it.
	/// </summary>
	static auto formatOperatorName(StringView opName_, Operator::Type type_) -> StaticString<char, 512>;

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
	auto findFunction(StringView funcName_) const -> FunctionOverloads const*;


	/// <summary>
	/// Tries to generate a function with name `funcName_` and parameter types `paramTypes_`
	/// out of the function templates registered in this scope.
	/// </summary>
	auto tryGenerateFunction(
			Instance&				vm_,
			StringView				funcName_,
			FunctionParamTypeSpan	paramTypes_
		) -> Function const*;

	/// <summary>
	/// Returns type with name `typeName_`,
	/// or `nullptr` if no such type exist within this scope.
	/// </summary>
	auto findType(StringView typeName_) const -> IType const*;

	/// <summary>
	/// Returns type with name `typeName_`,
	/// or `nullptr` if no such type exist within this or one of parent scopes.
	/// </summary>
	auto traceForType(StringView typeName_) const -> Opt< ScopeTraceResult<IType const*> >;

	/// <summary>
	/// Returns operator overload with name `opName_` and type `type_`,
	/// or `nullptr` if no such operator exist within this scope.
	/// </summary>
	/// <remarks>
	/// `formatOperatorName` is used within this function to get the correct name of an operator.
	/// </remarks>
	auto findOperator(StringView opName_, Operator::Type type_) const -> FunctionOverloads const*;

	/// <summary>
	/// Registers a type alias within this scope.
	/// </summary>
	auto registerType(Instance& vm_, StringView name_, IType& type_) -> IType&;

	/// <summary>
	/// Registers a function within this scope.
	/// </summary>
	auto registerFunction(Instance& vm_, StringView name_, Function func_) -> Function&;

	/// <summary>
	/// Registers a function template within this scope.
	/// </summary>
	auto registerFunctionTemplate(Instance& vm_, StringView name_, Function func_) -> Function&;

	/// <summary>
	/// Registers an operator within this scope.
	/// </summary>
	auto registerOperator(Instance& vm_, StringView name_, Operator::Type type_, Function func_) -> Function&;
};

auto setupUniverseScope(Instance &vm_, Scope& scope_) -> void;
}
