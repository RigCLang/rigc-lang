#pragma once

#include RIGCVM_PCH

#include <RigCVM/Value.hpp>
#include <RigCVM/Module.hpp>
#include <RigCVM/Scope.hpp>
#include <RigCVM/Stack.hpp>

#include <RigCVM/Functions.hpp>


namespace rigc::vm
{
class ClassType;
class StructuralType;

struct EntryPoint
{
	constexpr static auto DefaultFunctionName = std::string_view("main");

	/// Module that contains the entry point.
	Module* module_ = nullptr;

	/// Name of the function that is executed first when the script is run.
	std::string functionName = std::string(DefaultFunctionName);
};

struct Instance
{
	constexpr static auto StackSize			= std::size_t(2 * 1024 * 1024); // 2MB


	auto run(std::string_view moduleName_) -> int;

	auto executeFunction(Function const& func) -> OptValue;
	auto executeFunction(Function const& func, Function::ArgSpan args_) -> OptValue;
	auto evaluate(rigc::ParserNode const& stmt_) -> OptValue;

	/// <summary>Returns the "self" reference in method context.</summary>
	auto getSelf() -> Value;

	auto evaluateType(rigc::ParserNode const& typeNode_) -> DeclType;

	auto findVariableByName(std::string_view name_) -> OptValue;
	auto findType(std::string_view name_) -> IType const*;
	auto findFunction(std::string_view name_) -> FunctionCandidates;

	auto tryConvert(Value value_, DeclType const& to_) -> OptValue;

	auto cloneValue(Value value_) -> Value;

	/// Address is related to the code block memory obtained from a parser.
	auto scopeOf(void const *addr_) -> Scope&;

	/// Pushes the stack frame for specified address that is used to acquire a scope.
	/// Address is related to the code block memory obtained from a parser.
	auto pushStackFrameOf(void const* addr_) -> Scope&;

	/// Pops current stack frame
	auto popStackFrame() -> void;

	/// Returns the Universe Scope (the parent to the global scope).
	auto universalScope() -> Scope&
	{
		return *scopes[nullptr];
	}

	/// Allocates reference to a specified value.
	/// Note: when `toValue_` is a reference itself, it will create a reference to a reference.
	auto allocateReference(Value const& toValue_) -> Value ;

	/// Allocates pointer to a specified value.
	/// Note: `toRef_` must be a reference.
	auto allocatePointer(Value const& toRef_) -> Value ;

	auto reserveOnStack(DeclType type_, bool lookBack_ = false) -> FrameBasedValue ;

	/// Allocates stack space required for specified `type_`, initialized with value from `sourceBytes_`,
	/// by copying `sourceBytes_`.
	auto allocateOnStack(DeclType type_, void const* sourceBytes_, size_t toCopy = 0) -> Value;

	/// Allocates stack space required for specified `type_`, initialized with specified `value_`.
	template <typename T>
	auto allocateOnStack(DeclType type_, T const& value_) -> Value
	{
		return this->allocateOnStack(std::move(type_), reinterpret_cast<void const*>(&value_), sizeof(T));
	}

	/// Allocates stack space required for type specified by its `typeName_`, initialized with specified `value_`.
	template <typename T>
	auto allocateOnStack(std::string_view typeName_, T const& value_) -> Value
	{
		auto type = this->findType(typeName_);
		if (!type)
			throw std::runtime_error("Unknown type " + std::string(typeName_));

		return this->allocateOnStack<T>( type->shared_from_this(), value_ );
	}

	auto parseModule(std::string_view name_) -> Module*;

	auto evaluateModule(Module& module_) -> void;
	auto findModulePath(std::string_view name_) const -> fs::path;

	std::set<fs::path>						loadedModules;
	std::vector< std::shared_ptr<Module> >	modules;

	EntryPoint			entryPoint;

	/// Fixed-size memory pool used by emulated program to allocate
	/// temporary values.
	Stack				stack;

	/// Current execution scope, related to the current stack frame.
	Scope*				currentScope	= nullptr;

	/// Current execution scope, related to the current stack frame.
	FunctionInstance const*	currentFunc	= nullptr;

	/// Currently parsed class type.
	StructuralType*			currentClass	= nullptr;

	/// Currently executed method's class.
	ClassType const*	classContext	= nullptr;

	/// Whether currently executed function has triggered a return statement.
	bool				returnTriggered	= false;

	/// The level of loop breakage triggered by `break <number>;` or `break;`
	int				breakLevel	= 0;

	/// Whether currently executed loop has triggered a return statement.
	bool				continueTriggered	= false;



	/// Maps memory address to a related scope.
	/// Address might come from a parsed code (ParserNode)
	std::map<void const*, std::unique_ptr<Scope>>	scopes;

	size_t lineAt(rigc::ParserNode const& node_) const;
	size_t lastEvaluatedLine = 0;
private:
	rigc::ParserNode const* root = nullptr;
};

/// <summary>
/// Finds the first child of the node of the given type
/// </summary>
template <typename T>
inline auto findElem(rigc::ParserNode const& node_, bool recursive_ = true) -> rigc::ParserNode*
{
	auto it = rg::find_if(node_.children, &rigc::ParserNode::is_type<T>);

	if (it == node_.children.end())
	{
		if (!recursive_) return nullptr;
		else {
			for (auto const& child : node_.children)
			{
				rigc::ParserNode* result = findElem<T>(*child, true);
				if (result)
					return result;
			}
		}
		return nullptr;
	}

	return it->get();
}

/// <summary>
/// Finds the nth child of the node of the given type
/// </summary>
template <typename T>
inline auto findNthElem(rigc::ParserNode const& node_, std::size_t nth_) -> rigc::ParserNode*
{
	if(nth_ == 0) return nullptr;

	auto it = rg::find_if(node_.children, &rigc::ParserNode::is_type<T>);
	nth_--;

	while(nth_--) {
		it = rg::find_if(it + 1, node_.children.end(), &rigc::ParserNode::is_type<T>);
		if(it == node_.children.end()) return nullptr;
	}

	return it->get();
}

}
