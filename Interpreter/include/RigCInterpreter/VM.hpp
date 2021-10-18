#pragma once

#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Value.hpp>
#include <RigCInterpreter/Scope.hpp>

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

	Value* findVariableByName(std::string_view name_);
	TypeBase const* findType(std::string_view name_);
	FunctionOverloads const* findFunction(std::string_view name_);

	void	createVariable(std::string_view name_,	Value value_);
	Value	cloneVariable(std::string_view name_,	Value value_);
	Value	cloneValue(Value value_);

	Scope& pushScope() {
		auto scope = std::make_unique<Scope>();
		auto* scopePtr = scope.get();

		scope->initialStackSize = stackSize;
		scopes.push_back(std::move(scope));

		return *scopePtr;
	}

	void popScope()
	{
		// 2 because of the universe scope
		if (scopes.size() >= 2)
		{
			stackSize = scopes.back()->initialStackSize;

			scopes.erase(scopes.begin() + scopes.size() - 1);
		}
	}

	bool returnTriggered = false;

	Scope& univeralScope() {
		return *scopes[0];
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

	Value allocateOnStack(DeclType const& type_, void const* sourceBytes_, size_t toCopy = 0)
	{
		size_t toAlloc = type_.size();
		if (toCopy == 0)
			toCopy = toAlloc;
			
		size_t newSize = stackSize + toAlloc;
		if (newSize > STACK_SIZE)
			throw std::runtime_error("Stack overflow");

		size_t prevSize = stackSize;
		stackSize = newSize;

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

	void shrinkStack(size_t newSize)
	{
		stack.resize(newSize);
	}

	std::vector<char>	stack;
	// Do not use stack.size(), resize etc to
	// be 100% sure that it won't reallocate
	size_t				stackSize = 0;

	// std::stack<Value> stack;
	std::vector< std::unique_ptr<Scope> >		scopes;
	std::vector< std::unique_ptr<TypeBase> >	types;
	std::vector< std::unique_ptr<Function> >	functions;
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