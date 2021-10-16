#pragma once

#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Value.hpp>
#include <RigCInterpreter/Scope.hpp>

namespace rigc::vm
{
struct Instance;
using GlobalFunctions = std::vector< rigc::ParserNode *>;

struct Instance
{
	int run(rigc::ParserNodePtr & root);

	void executeFunction(rigc::ParserNode& func_);
	OptValue evaluate(rigc::ParserNode& stmt_);

	Value* findVariableByName(std::string_view name_);

	void createVariable(std::string_view name_, Value value_);

	Scope& pushScope() {
		auto scope = std::make_unique<Scope>();
		auto* scopePtr = scope.get();

		scopes.push_back(std::move(scope));

		return *scopePtr;
	}

	void popScope()
	{
		if (!scopes.empty())
		{
			scopes.erase(scopes.begin() + scopes.size() - 1);
		}
	}

	std::stack<Value> stack;
	std::vector< std::unique_ptr<Scope> > scopes;
private:

	GlobalFunctions discoverGlobalFunctions(rigc::ParserNodePtr& root);	
};

int runProgram(rigc::ParserNodePtr & root);

template <typename T>
rigc::ParserNode* findElem(rigc::ParserNode & node, bool recursive = true)
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