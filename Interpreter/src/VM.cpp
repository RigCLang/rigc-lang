#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/VM.hpp>

#include <RigCInterpreter/Executors/All.hpp>
#include <RigCInterpreter/Value.hpp>
#include <RigCInterpreter/TypeSystem/ClassTemplate.hpp>

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
Value Instance::allocateReference(Value const& toValue_)
{
	auto stackPtr = stack.container.data();
	auto stackPos = reinterpret_cast<char const*>(toValue_.data) - reinterpret_cast<char const*>(stackPtr);
	// fmt::print("Allocating reference to {}, type: {}\n", stackPos, toValue_.type->name());
	return this->allocateOnStack<void const*>(wrap<RefType>(univeralScope().types, toValue_.type), toValue_.blob());
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

	// Raw function:
	if (std::holds_alternative<Function::RawFn>(func_.impl))
	{
		// Process parameters (conversions)
		for (size_t i = 0; i < func_.paramCount; ++i)
		{
			auto& param = func_.params[i];
			// TODO: allow conversions, not only refs
			if (param.type != args_[i].type)
				args_[i] = args_[i].deref();
		}

		auto const& fn = std::get<Function::RawFn>(func_.impl);
		retVal = fn(*this, args_, argsCount_);
	}
	else
	{
		Scope& fnScope = this->pushScope(func_.addr());
		fnScope.func = true;

		// Process parameters (conversions)
		for (size_t i = 0; i < func_.paramCount; ++i)
		{
			auto& param = func_.params[i];

			// TODO: allow conversions, not only refs
			if (param.type != args_[i].type)
			{
				// fmt::print("Converting {} to {}\n", args_[i].type->name(), param.type->name());
				this->cloneValue(args_[i].deref());
			}
			else
				this->cloneValue(args_[i]);

			if (!fnScope.variables.contains(param.name))
			{
				fnScope.variables[std::string(param.name)] = this->reserveOnStack(param.type, true);
			}
		}
		// Runtime function:
		auto const& fn = *std::get<Function::RuntimeFn>(func_.impl);

		if (fn.is_type<rigc::FunctionDefinition>()) {
			retVal = this->evaluate( *findElem<rigc::CodeBlock>(fn) );
		}
		else if (fn.is_type<rigc::MethodDef>()) {
			retVal = this->evaluate( *findElem<rigc::CodeBlock>(fn) );
		}
		else if(fn.is_type<rigc::ClosureDefinition>()) {
			auto body = findElem<rigc::CodeBlock>(fn);
			if (!body)
				body = findElem<rigc::Expression>(fn);

			retVal = this->evaluate( *body );
		}

		this->popScope();
	}
	this->returnTriggered = false;

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
		return this->allocateOnStack( "Int32", size );
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
IType* Instance::findType(std::string_view name_)
{
	auto scope = currentScope;
	while (scope)
	{
		if (auto type = scope->types.find(name_))
			return type.get();

		scope = scope->parent;
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
	return this->allocateOnStack( value_.getType(), reinterpret_cast<void*>(value_.blob()), value_.getType()->size() );
}

//////////////////////////////////////////
Function& Instance::registerFunction(Function func_)
{
	auto f = std::make_unique<Function>( std::move(func_) );
	auto& ref = *f;
	functions.push_back(std::move(f));
	return ref;
}

//////////////////////////////////////////
IType& Instance::registerType(IType& type_)
{
	types.push_back( &type_ );
	return type_;
}

//////////////////////////////////////////
FrameBasedValue Instance::reserveOnStack(DeclType const& type_, bool lookBack_)
{
	auto& frame = stack.frames.back();

	FrameBasedValue result;
	result.type			= type_;
	result.stackOffset	= currentScope->baseFrameOffset;
	if (lookBack_)
		result.stackOffset -= type_->size();
	else
		currentScope->baseFrameOffset += type_->size();

	return result;
}

//////////////////////////////////////////
Value Instance::allocateOnStack(DeclType const& type_, void const* sourceBytes_, size_t toCopy)
{
	size_t toAlloc = type_->size();
	if (toCopy == 0)
		toCopy = toAlloc;

	size_t newSize = stack.size + toAlloc;
	if (newSize > STACK_SIZE)
		throw std::runtime_error("Stack overflow");

	size_t prevSize = stack.size;
	stack.size = newSize;

	currentScope->baseFrameOffset += toAlloc;

	char* bytes = stack.data() + prevSize;
	if (sourceBytes_)
		std::memcpy(bytes, sourceBytes_, toCopy);

	Value val;
	val.type = type_;
	val.data = bytes;
	return val;
}

//////////////////////////////////////////
Scope& Instance::scopeOf(void const *addr_)
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

//////////////////////////////////////////
Scope& Instance::pushScope(void const* addr_)
{
	Scope& scope = scopeOf(addr_);
	if (!scope.parent)
		scope.parent = currentScope;

	currentScope = &scope;
	StackFrame& frame = stack.pushFrame();
	frame.scope = &scope;
	return scope;
}

//////////////////////////////////////////
void Instance::popScope()
{
	// 2 because of the universe scope stack frame
	if (stack.frames.size() >= 2)
	{
		stack.popFrame();
		currentScope = stack.frames.back().scope;
	}
}


}
