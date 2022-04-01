#pragma once

#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Value.hpp>
#include <RigCInterpreter/Scope.hpp>
#include <RigCInterpreter/Stack.hpp>

#include <RigCInterpreter/Functions.hpp>

namespace rigc::vm
{
struct Instance;
using GlobalFunctions = std::vector< rigc::ParserNode *>;
class ClassType;

struct Instance
{
	constexpr static size_t STACK_SIZE = 2 * 1024 * 1024; // 2MB

	int run(rigc::ParserNodePtr & root);

	OptValue executeFunction(Function const& func);
	OptValue executeFunction(Function const& func, Function::Args& args_, size_t argsCount_=0);
	OptValue evaluate(rigc::ParserNode const& stmt_);

	OptValue findVariableByName(std::string_view name_);
	IType* findType(std::string_view name_);
	FunctionOverloads const* findFunction(std::string_view name_);

	Function const* findConversion(DeclType const& from_, DeclType const& to_);
	OptValue tryConvert(Value value_, DeclType const& to_);

	Value cloneValue(Value value_);

	Scope& scopeOf(void const *addr_);

	Scope& pushScope(void const* addr_);

	void popScope();

	Scope& univeralScope() {
		return *scopes[nullptr];
	}

	Function& registerFunction(Function func_);

	IType& registerType(IType& type_);

	FrameBasedValue reserveOnStack(DeclType const& type_, bool lookBack_ = false);

	Value allocateOnStack(DeclType const& type_, void const* sourceBytes_, size_t toCopy = 0);

	Value allocateReference(Value const& toValue_);

	template <typename T>
	Value allocateOnStack(DeclType const& type_, T const& value)
	{
		return this->allocateOnStack(type_, reinterpret_cast<void const*>(&value), sizeof(T));
	}

	template <typename T>
	Value allocateOnStack(std::string_view typeName_, T const& value)
	{
		IType* type = this->findType(typeName_);
		if (!type)
			throw std::runtime_error("Unknown type " + std::string(typeName_));

		return this->allocateOnStack<T>( type->shared_from_this(), value );
	}

	ClassType*			currentClass	= nullptr;
	bool				returnTriggered	= false;
	Stack				stack;
	// Do not use stack.size(), resize etc to
	// be 100% sure that it won't reallocate
	size_t				stackSize = 0;
	Scope*				currentScope = nullptr;
	TypeRegistry									typeRegistry;
	std::map<void const*, std::unique_ptr<Scope>>	scopes;
	std::vector< IType* >							types;
	std::vector< std::unique_ptr<Function> >		functions;
private:

	GlobalFunctions discoverGlobalFunctions(rigc::ParserNodePtr& root);
};

int runProgram(rigc::ParserNodePtr & root);

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
