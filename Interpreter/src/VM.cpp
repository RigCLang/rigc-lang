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

#define DEFINE_BUILTIN_CONVERT_OP(ToCppType, ToRuntimeType)									\
	template <typename FromType>															\
	OptValue builtinConvertOperator_##ToRuntimeType(Instance &vm_, Value const& lhs_)		\
	{																						\
		FromType const&	lhsData = *reinterpret_cast<FromType const*>(lhs_.blob());			\
		return vm_.allocateOnStack<ToCppType>(#ToRuntimeType, ToCppType(lhsData));			\
	}

DEFINE_BUILTIN_CONVERT_OP	(int16_t,	Int16);
DEFINE_BUILTIN_CONVERT_OP	(int32_t,	Int32);
DEFINE_BUILTIN_CONVERT_OP	(int64_t,	Int64);

DEFINE_BUILTIN_CONVERT_OP	(float,		Float32);
DEFINE_BUILTIN_CONVERT_OP	(double,	Float64);

#undef DEFINE_BUILTIN_CONVERT_OP

//////////////////////////////////////////
int Instance::run(rigc::ParserNodePtr& root)
{
	stack.container.resize(STACK_SIZE);
	scopes[nullptr] = makeUniverseScope(*this);
	Scope& scope = *scopes[nullptr];
	this->pushScope(nullptr);

	#define ADD_CONVERSION(FromCppType, FromRigCName, ToRigCName) \
		addTypeConversion<FromCppType>(*this, scope, #FromRigCName, #ToRigCName, builtinConvertOperator_##ToRigCName<FromCppType>)


	// // Int16 -> floats
	ADD_CONVERSION(int16_t,	Int16,		Float32);
	ADD_CONVERSION(int16_t,	Int16,		Float64);

	// // Int32 -> floats
	ADD_CONVERSION(int32_t,	Int32,		Float32);
	ADD_CONVERSION(int32_t,	Int32,		Float64);

	// // Int64 -> floats
	ADD_CONVERSION(int64_t,	Int64,		Float32);
	ADD_CONVERSION(int64_t,	Int64,		Float64);

	// // Float32 -> ints
	ADD_CONVERSION(float,	Float32,	Int16);
	ADD_CONVERSION(float,	Float32,	Int32);
	ADD_CONVERSION(float,	Float32,	Int64);

	// // Float64 -> ints
	ADD_CONVERSION(double,	Float64,	Int16);
	ADD_CONVERSION(double,	Float64,	Int32);
	ADD_CONVERSION(double,	Float64,	Int64);

	#undef ADD_CONVERSION


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
	Scope& fnScope = this->pushScope(func_.addr());
	fnScope.func = true;

	// TODO: push parameters
	for (size_t i = 0; i < func_.paramCount; ++i)
	{
		this->cloneValue(args_[i]);

		auto& param = func_.params[i];
		if (!fnScope.variables.contains(param.name))
		{
			fnScope.variables[std::string(param.name)] = this->reserveOnStack(param.type, true);
		}
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
	constexpr std::string_view prefix = "struct rigc::";

	auto it = Executors.find( stmt_.type.substr( prefix.size() ));
	if (it != Executors.end())
		return it->second(*this, stmt_);

	std::cout << "No executors for \"" << stmt_.type << "\": " << stmt_.string_view() << std::endl;
	return {};
}

//////////////////////////////////////////
Function const* Instance::findConversion(DeclType const& from_, DeclType const& to_)
{
	return this->univeralScope().findConversion(from_, to_);
}

//////////////////////////////////////////
OptValue Instance::tryConvert(Value value_, DeclType const& to_)
{
	if (value_.type == to_)
		return value_;

	auto cvt = this->findConversion(value_.type, to_);
	if (!cvt)
		return std::nullopt;

	Function::Args args;
	args[0] = value_;
	return this->executeFunction(*cvt, args, 1);
}

//////////////////////////////////////////
OptValue Instance::findVariableByName(std::string_view name_)
{
	if (name_ == "stackSize")
	{
		int size = static_cast<int>(stack.size);
		return this->allocateOnStack( DeclType::fromType( *findType("Int32") ), reinterpret_cast<void*>(&size), sizeof(int) );
	}

	for (auto it = stack.frames.rbegin(); it != stack.frames.rend(); )
	{
		auto& vars = it->scope->variables;
		auto varIt = vars.find(name_);

		if (varIt != vars.end())
		{
			return varIt->second.toAbsolute(*it);
		}

		if (it->scope->func)
			it = stack.frames.rend() - 1;
		else
			++it;
	}

	return std::nullopt;
}

//////////////////////////////////////////
TypeBase const* Instance::findType(std::string_view name_)
{
	for (auto it = stack.frames.rbegin(); it != stack.frames.rend(); ++it)
	{
		auto& types = it->scope->typeAliases;
		auto typeIt = types.find(name_);

		if (typeIt != types.end())
			return typeIt->second;
	}

	return nullptr;
}

//////////////////////////////////////////
FunctionOverloads const* Instance::findFunction(std::string_view name_)
{
	for (auto it = stack.frames.rbegin(); it != stack.frames.rend(); ++it)
	{
		auto& funcs = it->scope->functions;
		auto funcIt = funcs.find(name_);

		if (funcIt != funcs.end())
			return &funcIt->second;

	}

	return nullptr;
}

//////////////////////////////////////////
Value Instance::cloneValue(Value value_)
{
	return this->allocateOnStack( value_.getType(), reinterpret_cast<void*>(value_.blob()), value_.getType().size() );
}


}
