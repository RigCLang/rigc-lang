#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/VM.hpp>

#include <RigCInterpreter/Executors/All.hpp>
#include <RigCInterpreter/Value.hpp>
#include <RigCInterpreter/TypeSystem/ClassTemplate.hpp>
#include <RigCInterpreter/TypeSystem/ClassType.hpp>
#include <RigCInterpreter/TypeSystem/RefType.hpp>
#include <RigCInterpreter/TypeSystem/ArrayType.hpp>
#include <RigCInterpreter/TypeSystem/FuncType.hpp>

namespace rigc::vm
{

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
fs::path Instance::findModulePath(std::string_view name_) const
{
	fs::path path = std::string(name_);

	if (!path.has_extension())
	{
		path += ".rigc";
		if (!fs::exists(path))
		{
			path = std::string(name_) + ".rigcz";
			if (!fs::exists(path))
				return fs::path{};
		}
	}

	return fs::absolute(path);
}

//////////////////////////////////////////
Module* Instance::parseModule(std::string_view name_)
{
	auto path = this->findModulePath(name_);

	loadedModules.insert(path);

	auto mod		= std::make_unique<Module>(*this);
	mod->fileInput	= std::make_unique<pegtl::file_input<>>(path);
	mod->root		= rigc::parse( *mod->fileInput );

	if (!mod->root)
		return nullptr;

	mod->absolutePath = path;

	modules.emplace_back(std::move(mod));
	return modules.back().get();
}

//////////////////////////////////////////
void Instance::evaluateModule(Module& module_)
{
	for (auto const& stmt : module_.root->children)
	{
		this->evaluate(*stmt);
	}
}

//////////////////////////////////////////
int Instance::run(std::string_view moduleName_)
{
	entryPoint.module_ = this->parseModule(moduleName_);
	if (!entryPoint.module_)
	{
		throw std::runtime_error(fmt::format("Failed to run module \"{}\"", moduleName_));
	}

	stack.container.resize(StackSize);
	scopes[nullptr] = makeUniverseScope(*this);
	Scope& scope = *scopes[nullptr];
	this->pushStackFrameOf(nullptr);

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

	this->evaluateModule(*entryPoint.module_);

	auto mainFuncOv = this->universalScope().findFunction(entryPoint.functionName);

	if (!mainFuncOv)
		throw std::runtime_error(fmt::format("Cannot execute script. Function \"{}\" not found.", entryPoint.functionName));

	if (mainFuncOv->size() > 1)
		throw std::runtime_error(fmt::format("Entry point function \"{}\" cannot be overloaded.", entryPoint.functionName));

	this->executeFunction(*(*mainFuncOv)[0]);

	return 0;
}

//////////////////////////////////////////
Value Instance::allocateReference(Value const& toValue_)
{
	return this->allocateOnStack<void const*>(wrap<RefType>(universalScope(), toValue_.type), toValue_.blob());
}

//////////////////////////////////////////
Value Instance::allocatePointer(Value const& toRef_)
{
	auto deref = toRef_.removeRef();
	return this->allocateOnStack<void const*>(wrap<AddrType>(universalScope(), deref.type), deref.blob());
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

	auto prevClassContext = classContext;

	if (func_.outerType && func_.outerType->is<ClassType>())
		classContext = func_.outerType->as<ClassType>();

	bool rawFn = std::holds_alternative<Function::RawFn>(func_.impl);
	// Raw function:
	if (rawFn)
	{
		// Process parameters (conversions)
		for (size_t i = 0; i < func_.paramCount; ++i)
		{
			auto& param = func_.params[i];
			// TODO: allow conversions, not only refs
			if (param.type != args_[i].type)
				args_[i] = args_[i].removeRef();
		}

		auto const& fn = std::get<Function::RawFn>(func_.impl);
		retVal = fn(*this, args_, argsCount_);
	}
	else
	{
		size_t prevStackFrames = stack.frames.size();
		Scope& fnScope = this->pushStackFrameOf(func_.addr());
		StackFrame& frame = stack.frames.back();
		fnScope.func = true;

		// Process parameters (conversions)
		for (size_t i = 0; i < func_.paramCount; ++i)
		{
			auto& param = func_.params[i];

			// TODO: allow conversions, not only refs
			if (param.type != args_[i].type)
			{
				this->cloneValue(args_[i].removeRef());
			}
			else
				this->cloneValue(args_[i]);

			if (!fnScope.variables.contains(param.name))
			{
				auto paramName = std::string(param.name);
				auto& var = fnScope.variables[paramName];
				var = this->reserveOnStack(param.type, true);
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

		while (stack.frames.size() > prevStackFrames)
			this->popStackFrame();
	}
	this->returnTriggered = false;

	classContext = prevClassContext;

	if (retVal.has_value())
	{
		Value val;
		if (!func_.returnsRef && retVal->type->is<RefType>())
			val = this->cloneValue(retVal->removeRef());
		else
			val = this->cloneValue(*retVal);

		return val; // extend lifetime
	}

	return {};
}

//////////////////////////////////////////
OptValue Instance::evaluate(rigc::ParserNode const& stmt_)
{
	lastEvaluatedLine = this->lineAt(stmt_);

	constexpr std::string_view prefix = "struct rigc::";
	auto it = Executors.find( stmt_.type.substr( prefix.size() ));
	if (it != Executors.end())
	{
		auto val = it->second(*this, stmt_);
		return val;
	}

	fmt::print("No executors for \"{}\": {}\n", stmt_.type, stmt_.string_view());
	return {};
}

//////////////////////////////////////////
OptValue Instance::tryConvert(Value value_, DeclType const& to_)
{
	if (value_.type == to_)
		return value_;

	auto cvt = this->universalScope().findConversion(value_.type, to_);
	if (!cvt)
		return std::nullopt;

	Function::Args args;
	args[0] = value_;
	return this->executeFunction(*cvt, args, 1);
}

//////////////////////////////////////////
Value Instance::getSelf()
{
	assert(classContext && "Cannot get self reference outside a method");

	return *this->findVariableByName("self");
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
		{
			// If within class, search for a class data member
			if (classContext)
			{
				auto& dataMembers = classContext->dataMembers;

				auto dataMemberIt = rg::find(dataMembers, name_, &DataMember::name);

				if (dataMemberIt != dataMembers.end())
				{
					return this->getSelf().removeRef().member(dataMemberIt->offset, dataMemberIt->type);
				}
			}

			// Jump to the global scope
			it = stack.frames.rend() - 1;
		}
		else
			++it;
	}

	return std::nullopt;
}

//////////////////////////////////////////
size_t Instance::lineAt(rigc::ParserNode const& node_) const
{
	return node_.m_begin.line;
}

//////////////////////////////////////////
DeclType Instance::evaluateType(rigc::ParserNode const& typeNode_)
{
	DeclType evaluatedType;
	auto typeName		= findElem<rigc::Name>(typeNode_)->string_view();
	auto templateParams = findElem<rigc::TemplateParams>(typeNode_);

	if (templateParams)
	{
		if (typeName == "Ref")
		{
			auto inner = findElem<rigc::TemplateParam>(*templateParams);
			return wrap<RefType>(
					this->universalScope(),
					this->evaluateType(*findElem<rigc::Type>(*inner))
				);
		}
		else if (typeName == "Addr")
		{
			auto inner = findElem<rigc::TemplateParam>(*templateParams);
			return wrap<AddrType>(
					this->universalScope(),
					this->evaluateType(*findElem<rigc::Type>(*inner))
				);
		}
		else if (typeName == "StaticArray")
		{
			// ensure 2 template params
			if (templateParams->children.size() != 2)
				throw std::runtime_error("StaticArray requires 2 template params: StaticArray<T, Int32 N>");

			auto inner	= this->evaluateType(*findElem<rigc::Type>(*templateParams->children[0]));
			// TEMP:
			auto size	= std::stoi(templateParams->children[1]->string());

			return wrap<ArrayType>(this->universalScope(), inner, size);
		}
	}

	return this->findType(typeName)->shared_from_this();
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
Value Instance::findFunctionExpr(std::string_view name_)
{
	FunctionOverloads const* overloads = nullptr;
	IType* type = nullptr;

	if (auto constructed = this->findType(name_))
	{
		if (auto c = constructed->as<ClassType>())
		{
			overloads	= c->constructors();
			return allocateMethodOverloads(*this, {}, overloads);
		}
	}

	overloads	= this->findFunction(name_);
	type		= this->findType(BuiltinTypes::OverloadedFunction);

	return this->allocateOnStack<void const*>(type->shared_from_this(), overloads);
}

//////////////////////////////////////////
Value Instance::cloneValue(Value value_)
{
	return this->allocateOnStack( value_.getType(), reinterpret_cast<void*>(value_.blob()), value_.getType()->size() );
}

//////////////////////////////////////////
FrameBasedValue Instance::reserveOnStack(DeclType type_, bool lookBack_)
{
	auto& frame = stack.frames.back();

	auto size = type_->size();

	FrameBasedValue result;
	result.type			= std::move(type_);
	result.stackOffset	= stack.size - frame.initialStackSize;
	if (lookBack_)
		result.stackOffset -= size;
	else
		stack.size += size;

	return result;
}

//////////////////////////////////////////
Value Instance::allocateOnStack(DeclType type_, void const* sourceBytes_, size_t toCopy)
{
	size_t toAlloc = type_->size();
	if (toCopy == 0)
		toCopy = toAlloc;

	size_t newSize = stack.size + toAlloc;
	if (newSize > StackSize)
		throw std::runtime_error("Stack overflow");

	size_t prevSize = stack.size;
	stack.size = newSize;

	char* bytes = stack.data() + prevSize;
	if (sourceBytes_)
		std::memcpy(bytes, sourceBytes_, toCopy);

	Value val;
	val.type = std::move(type_);
	val.data = bytes;
	return val;
}

//////////////////////////////////////////
Scope& Instance::scopeOf(void const *addr_)
{
	auto it = scopes.find(addr_);
	if (it == scopes.end())
	{
		auto scope = std::make_unique<Scope>(*this);
		auto& scopeRef = *scope;
		scopes[addr_] = std::move(scope);
		return scopeRef;
	}

	return *scopes[addr_];
}

//////////////////////////////////////////
Scope& Instance::pushStackFrameOf(void const* addr_)
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
void Instance::popStackFrame()
{
	assert(stack.frames.size() > 1 && "Tried to pop a universe scope-related stack frame.");

	stack.popFrame();
	currentScope = stack.frames.back().scope;
}


}
