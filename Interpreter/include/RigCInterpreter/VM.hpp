#pragma once

#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Value.hpp>
#include <RigCInterpreter/Module.hpp>
#include <RigCInterpreter/Scope.hpp>
#include <RigCInterpreter/Stack.hpp>

#include <RigCInterpreter/Functions.hpp>


namespace rigc::vm
{
class ClassType;

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


	int run(std::string_view moduleName_);

	OptValue executeFunction(Function const& func);
	OptValue executeFunction(Function const& func, Function::Args& args_, size_t argsCount_=0);
	OptValue evaluate(rigc::ParserNode const& stmt_);

	/// <summary>Returns the "self" reference in method context.</summary>
	Value getSelf();

	OptValue findVariableByName(std::string_view name_);
	IType* findType(std::string_view name_);
	FunctionOverloads const* findFunction(std::string_view name_);
	Value findFunctionExpr(std::string_view name_);

	OptValue tryConvert(Value value_, DeclType const& to_);

	Value cloneValue(Value value_);

	/// Address is related to the code block memory obtained from a parser.
	Scope& scopeOf(void const *addr_);

	/// Pushes the stack frame for specified address that is used to acquire a scope.
	/// Address is related to the code block memory obtained from a parser.
	Scope& pushStackFrameOf(void const* addr_);

	/// Pops current stack frame
	void popStackFrame();

	/// Returns the Universe Scope (the parent to the global scope).
	Scope& universalScope() {
		return *scopes[nullptr];
	}

	/// Allocates reference to a specified value.
	/// Note: when `toValue_` is a reference itself, it will create a reference to a reference.
	Value allocateReference(Value const& toValue_);

	/// Allocates pointer to a specified value.
	/// Note: `toRef_` must be a reference.
	Value allocatePointer(Value const& toRef_);

	FrameBasedValue reserveOnStack(DeclType type_, bool lookBack_ = false);

	/// Allocates stack space required for specified `type_`, initialized with value from `sourceBytes_`,
	/// by copying `sourceBytes_`.
	Value allocateOnStack(DeclType type_, void const* sourceBytes_, size_t toCopy = 0);

	/// Allocates stack space required for specified `type_`, initialized with specified `value_`.
	template <typename T>
	Value allocateOnStack(DeclType type_, T const& value_)
	{
		return this->allocateOnStack(std::move(type_), reinterpret_cast<void const*>(&value_), sizeof(T));
	}

	/// Allocates stack space required for type specified by its `typeName_`, initialized with specified `value_`.
	template <typename T>
	Value allocateOnStack(std::string_view typeName_, T const& value_)
	{
		IType* type = this->findType(typeName_);
		if (!type)
			throw std::runtime_error("Unknown type " + std::string(typeName_));

		return this->allocateOnStack<T>( type->shared_from_this(), value_ );
	}

	Module* parseModule(std::string_view name_);

	void evaluateModule(Module& module_);
	fs::path findModulePath(std::string_view name_) const;

	std::set<fs::path>						loadedModules;
	std::vector< std::shared_ptr<Module> >	modules;

	EntryPoint			entryPoint;

	/// Fixed-size memory pool used by emulated program to allocate
	/// temporary values.
	Stack				stack;

	/// Current execution scope, related to the current stack frame.
	Scope*				currentScope	= nullptr;

	/// Current execution scope, related to the current stack frame.
	rigc::ParserNode const*	currentFunc	= nullptr;

	/// Currently parsed class type.
	ClassType*			currentClass	= nullptr;

	/// Currently executed method's class.
	ClassType const*	classContext	= nullptr;

	/// Whether currently executed function has triggered a return statement.
	bool				returnTriggered	= false;



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

}
