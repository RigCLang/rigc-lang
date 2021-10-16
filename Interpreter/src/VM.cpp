#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/VM.hpp>

#include <RigCInterpreter/Executors/All.hpp>

namespace rigc::vm
{


//////////////////////////////////////////
std::string_view funcName(rigc::ParserNode & func_)
{
	auto name = findElem<rigc::Name>(func_, false);
	
	if (name)
		return name->string_view();
	else
		return {};
}

//////////////////////////////////////////
int runProgram(rigc::ParserNodePtr & root)
{
	Instance instance;
	return instance.run(root);
}

//////////////////////////////////////////
int Instance::run(rigc::ParserNodePtr & root)
{
	auto functions = this->discoverGlobalFunctions(root);

	auto mainFunc = rg::find_if(functions,
		[](auto const& func) {
			auto const& c = func->children;
			return (funcName(*func) == "main");
		});

	if (mainFunc != functions.end())
	{
		this->executeFunction(*(*mainFunc));
	}

	return 0;
}

//////////////////////////////////////////
GlobalFunctions Instance::discoverGlobalFunctions(rigc::ParserNodePtr & root)
{
	GlobalFunctions result;
	result.reserve(1024);

	for (auto& elem : root->children)
	{
		if (elem->is_type<rigc::FunctionDefinition>())
			result.push_back(elem.get());
	}

	return result;
}

//////////////////////////////////////////
void Instance::executeFunction(rigc::ParserNode& func_)
{
	this->pushScope();

	// std::cout << "Executing " << funcName(func_) << std::endl;

	this->evaluate( *findElem<rigc::CodeBlock>(func_) );

	this->popScope();
}

//////////////////////////////////////////
OptValue Instance::evaluate(rigc::ParserNode& stmt_)
{
	auto it = Executors.find(stmt_.type);
	if (it != Executors.end())
		return it->second(*this, stmt_);
	
	std::cout << "No executors for \"" << stmt_.type << "\": " << stmt_.string_view() << std::endl;
	return {};
}

//////////////////////////////////////////
Value* Instance::findVariableByName(std::string_view name_)
{
	for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
	{
		auto& vars = it->get()->variables;
		auto varIt = vars.find(name_);

		if (varIt != vars.end())
			return &(varIt->second);
	}

	return nullptr;
}

//////////////////////////////////////////
void Instance::createVariable(std::string_view name_, Value value_)
{
	auto& vars = scopes.back()->variables;
	if (vars.find(name_) != vars.end())
	{
		throw std::runtime_error("Variable with name \"" + std::string(name_) + "\" already defined.");
	}

	vars[name_] = std::move(value_);
}


}