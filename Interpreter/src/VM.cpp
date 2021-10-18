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
int Instance::run(rigc::ParserNodePtr& root)
{
	stack.resize(STACK_SIZE);
	scopes.push_back( makeUniverseScope(*this) );

	for (auto const& stmt : root->children)
	{
		this->evaluate(*stmt);
	}

	auto mainFuncOv = this->univeralScope().findFunction("main");

	if (!mainFuncOv)
		throw std::runtime_error("\"main\" function not found.");

	this->executeFunction(*(*mainFuncOv)[0]);

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
OptValue Instance::executeFunction(Function const& func)
{
	Function::Args args;
	return this->executeFunction(func, args, 0);
}

//////////////////////////////////////////
OptValue Instance::executeFunction(Function const& func_, Function::Args& args_, size_t argsCount_)
{
	OptValue retVal;
	this->pushScope();

	// TODO: push parameters
	for (size_t i = 0; i < func_.paramCount; ++i)
	{
		this->cloneVariable(func_.params[i].name, args_[i]);
	}

	// Raw function:
	if (std::holds_alternative<Function::RawFn>(func_.impl))
	{
		auto const& fn = std::get<Function::RawFn>(func_.impl);
		retVal = fn(*this, args_, argsCount_);
	}
	else
	{
		// Runtime function:
		auto const& fn = *std::get<Function::RuntimeFn>(func_.impl);

		
		if (fn.is_type<rigc::FunctionDefinition>()) {
			retVal = this->evaluate( *findElem<rigc::CodeBlock>(fn) );
		}
		else if(fn.is_type<rigc::ClosureDefinition>()) {
			auto body = findElem<rigc::CodeBlock>(fn);
			if (!body)
				body = findElem<rigc::Expression>(fn);

			retVal = this->evaluate( *body );
		}
	}
	this->returnTriggered = false;
	this->popScope();

	if (retVal.has_value())
	{
		Value val = this->cloneValue(*retVal);
		return val; // extend lifetime
	}

	return {};
}

//////////////////////////////////////////
OptValue Instance::evaluate(rigc::ParserNode const& stmt_)
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
TypeBase const* Instance::findType(std::string_view name_)
{
	for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
	{
		auto& types = it->get()->typeAliases;
		auto typeIt = types.find(name_);

		if (typeIt != types.end())
			return typeIt->second;
	}

	return nullptr;
}

//////////////////////////////////////////
FunctionOverloads const* Instance::findFunction(std::string_view name_)
{
	for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
	{
		auto& funcs = it->get()->functions;
		auto funcIt = funcs.find(name_);

		if (funcIt != funcs.end())
			return &funcIt->second;

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

	vars[std::string(name_)] = std::move(value_);
}

//////////////////////////////////////////
Value Instance::cloneVariable(std::string_view name_, Value value_)
{
	Value val = this->cloneValue(value_);
	this->createVariable(name_, val);
	return val;
}

//////////////////////////////////////////
Value Instance::cloneValue(Value value_)
{
	return this->allocateOnStack( value_.getType(), reinterpret_cast<void*>(value_.blob()), value_.getType().size() );
}


}