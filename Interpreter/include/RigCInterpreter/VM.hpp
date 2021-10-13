#pragma once

#include RIGCINTERPRETER_PCH

namespace rigc::vm
{
struct Instance;
using GlobalFunctions = std::vector< rigc::ParserNode *>;


struct Value
{
	int32_t val;
};

using OptValue = std::optional<Value>;

struct Instance
{
	int run(rigc::ParserNodePtr & root);

	void executeFunction(rigc::ParserNode& func_);
	OptValue evaluate(rigc::ParserNode& stmt_);

	std::stack<Value> stack;
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