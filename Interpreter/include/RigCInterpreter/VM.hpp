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

struct Instance
{
	constexpr static size_t STACK_SIZE = 2 * 1024 * 1024; // 2MB

	int run(rigc::ParserNodePtr & root);

	OptValue executeFunction(Function const& func);
	OptValue executeFunction(Function const& func, Function::Args& args_, size_t argsCount_=0);
	OptValue evaluate(rigc::ParserNode const& stmt_);

	OptValue findVariableByName(std::string_view name_);
	TypeBase const* findType(std::string_view name_);
	FunctionOverloads const* findFunction(std::string_view name_);

	void	createVariable(std::string_view name_,	Value value_);
	Value	cloneVariable(std::string_view name_,	Value value_);
	Value	cloneValue(Value value_);

	Scope& scopeOf(void const *addr_)
	{
		auto it = scopes.find(addr_);
		if (it == scopes.end())
		{
			auto scope = std::make_unique<Scope>();
			auto& scopeRef = *scope;
			scopes[addr_] = std::move(scope);
			return scopeRef;
		}

		return *scopes[addr_];
	}

	Scope& pushScope(void const* addr_)
	{
		Scope& scope = scopeOf(addr_);
		currentScope = &scope;
		StackFrame& frame = stack.pushFrame();
		frame.scope = &scope;
		return scope;
	}

	void popScope()
	{
		// 2 because of the universe scope stack frame
		if (stack.frames.size() >= 2)
		{
			stack.popFrame();
			currentScope = stack.frames.back().scope;
		}
	}

	bool returnTriggered = false;

	Scope& univeralScope() {
		return *scopes[nullptr];
	}

	Function& registerFunction(Function func_)
	{
		auto f = std::make_unique<Function>( std::move(func_) );
		auto& ref = *f;
		functions.push_back(std::move(f));
		return ref;
	}

	TypeBase& registerType(TypeBase type_)
	{
		auto t = std::make_unique<TypeBase>( std::move(type_) );
		auto& ref = *t;
		types.push_back( std::move(t) );
		return ref;
	}

	FrameBasedValue reserveOnStack(DeclType const& type_, bool lookBack_ = false)
	{
		auto& frame = stack.frames.back();

		FrameBasedValue result;
		result.type			= type_;
		result.stackOffset	= currentScope->baseFrameOffset;
		if (lookBack_)
			result.stackOffset -= type_.size();
		else
			currentScope->baseFrameOffset += type_.size();

		return result;
	}

	Value allocateOnStack(DeclType const& type_, void const* sourceBytes_, size_t toCopy = 0)
	{
		size_t toAlloc = type_.size();
		if (toCopy == 0)
			toCopy = toAlloc;

		size_t newSize = stack.size + toAlloc;
		if (newSize > STACK_SIZE)
			throw std::runtime_error("Stack overflow");

		size_t prevSize = stack.size;
		stack.size = newSize;

		currentScope->baseFrameOffset += toAlloc;

		char* bytes = stack.data() + prevSize;
		std::memcpy(bytes, sourceBytes_, toCopy);

		Value val;
		val.type = type_;
		val.data = bytes;
		return val;
	}

	template <typename T>
	Value allocateOnStack(DeclType const& type_, T const& value)
	{
		return this->allocateOnStack(type_, reinterpret_cast<void const*>(&value), sizeof(T));
	}

	template <typename T>
	Value allocateOnStack(std::string_view typeName_, T const& value)
	{
		TypeBase const* type = this->findType(typeName_);
		if (!type)
			throw std::runtime_error("Unknown type " + std::string(typeName_));

		return this->allocateOnStack<T>( DeclType::fromType(*type), value );
	}

	Stack				stack;
	// Do not use stack.size(), resize etc to
	// be 100% sure that it won't reallocate
	size_t				stackSize = 0;

	Scope* currentScope = nullptr;

	std::map<void const*, std::unique_ptr<Scope>>	scopes;
	std::vector< std::unique_ptr<TypeBase> >		types;
	std::vector< std::unique_ptr<Function> >		functions;
private:

	GlobalFunctions discoverGlobalFunctions(rigc::ParserNodePtr& root);
};

int runProgram(rigc::ParserNodePtr & root);

template <typename T>
rigc::ParserNode* findElem(rigc::ParserNode const& node, bool recursive = true)
{
	auto it = rg::find_if(node.children,
		[&](auto const& child){
			return child->is_type<T>();
		});

	if (it == node.children.end())
	{
		if (!recursive) return nullptr;
		else {
			for (auto const& child : node.children)
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
