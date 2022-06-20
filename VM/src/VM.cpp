#include RIGCVM_PCH

#include <RigCVM/VM.hpp>

#include <RigCVM/Executors/All.hpp>
#include <RigCVM/Value.hpp>
#include <RigCVM/TypeSystem/ClassTemplate.hpp>
#include <RigCVM/TypeSystem/ClassType.hpp>
#include <RigCVM/TypeSystem/RefType.hpp>
#include <RigCVM/TypeSystem/ArrayType.hpp>
#include <RigCVM/TypeSystem/FuncType.hpp>

namespace rigc::vm
{

#define DEFINE_BUILTIN_CONVERT_OP(ToCppType, ToRuntimeType)									\
	template <typename FromType>															\
	auto builtinConvertOperator_##ToRuntimeType(Instance &vm_, Value const& lhs_) -> OptValue		\
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
auto Instance::findModulePath(std::string_view name_) const -> fs::path
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
auto Instance::parseModule(std::string_view name_) -> Module*
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
auto Instance::evaluateModule(Module& module_) -> void
{
	for (auto const& stmt : module_.root->children)
	{
		this->evaluate(*stmt);
	}
}

//////////////////////////////////////////
auto Instance::run(std::string_view moduleName_) -> int
{
	entryPoint.module_ = this->parseModule(moduleName_);
	if (!entryPoint.module_)
	{
		throw std::runtime_error(fmt::format("Failed to run module \"{}\"", moduleName_));
	}

	// Use its parent path as the working directory
	// This is important for modules to work properly.
	fs::current_path(entryPoint.module_->absolutePath.parent_path());

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
auto Instance::allocateReference(Value const& toValue_) -> Value
{
	return this->allocateOnStack<void const*>(constructTemplateType<RefType>(universalScope(), toValue_.type), toValue_.blob());
}

//////////////////////////////////////////
auto Instance::allocatePointer(Value const& toRef_) -> Value
{
	auto deref = toRef_.safeRemoveRef();
	return this->allocateOnStack<void const*>(constructTemplateType<AddrType>(universalScope(), deref.type), deref.blob());
}

//////////////////////////////////////////
auto Instance::executeFunction(Function const& func) -> OptValue
{
	return this->executeFunction(func, {});
}

//////////////////////////////////////////
auto Instance::executeFunction(Function const& func_, Function::ArgSpan args_) -> OptValue
{
	OptValue retVal;

	auto prevClassContext = classContext;

	if (func_.outerType && func_.outerType->is<ClassType>())
		classContext = func_.outerType->as<ClassType>();

	// Raw function:
	if (func_.isRaw())
	{
		// Process parameters (conversions)
		for (size_t i = 0; i < func_.paramCount; ++i)
		{
			auto& param = func_.params[i];
			// TODO: allow conversions, not only refs
			if (param.type != args_[i].type)
				args_[i] = args_[i].removeRef();
		}

		auto const& fn = func_.rawImpl();
		retVal = fn(*this, args_);
	}
	else
	{
		auto prevStackFrames	= stack.frames.size();
		auto& fnScope			= this->pushStackFrameOf(func_.addr());
		auto& frame				= stack.frames.back();

		fnScope.func = &func_;

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
		auto const& fn = *func_.runtimeImpl().node;

		if (fn.is_type<rigc::FunctionDefinition>() || fn.is_type<rigc::MethodDef>())
		{
			retVal = this->evaluate( *findElem<rigc::CodeBlock>(fn) );
		}
		// TODO: support closures
		// else if(fn.is_type<rigc::ClosureDefinition>())
		// {
		// 	auto body = findElem<rigc::CodeBlock>(fn);
		// 	if (!body)
		// 		body = findElem<rigc::Expression>(fn);

		// 	retVal = this->evaluate( *body );
		// }

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
auto Instance::evaluate(rigc::ParserNode const& stmt_) -> OptValue
{
	lastEvaluatedLine = this->lineAt(stmt_);

// FIXME: a quickfix
#ifdef _MSC_VER
	constexpr std::string_view prefix = "struct rigc::";
#elif defined(__GNUC__) || defined(__clang__)
	constexpr std::string_view prefix = "rigc::";
#endif

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
auto Instance::tryConvert(Value value_, DeclType const& to_) -> OptValue
{
	if (value_.type == to_)
		return value_;

	auto cvt = this->universalScope().findConversion(value_.type, to_);
	if (!cvt)
		return std::nullopt;

	Function::Args args;
	args[0] = value_;
	return this->executeFunction(*cvt, Function::ArgSpan{ args.data(), 1 });
}

//////////////////////////////////////////
auto Instance::getSelf() -> Value
{
	assert(classContext && "Cannot get self reference outside a method");

	return *this->findVariableByName("self");
}

//////////////////////////////////////////
auto Instance::findVariableByName(std::string_view name_) -> OptValue
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

		auto& templArgs = it->scope->templateArguments;
		auto templConstantIt = templArgs.find(name_);
		if (templConstantIt != templArgs.end())
		{
			if (templConstantIt->second.is<int>())
				return this->allocateOnStack( "Int32", templConstantIt->second.as<int>() );
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
auto Instance::lineAt(rigc::ParserNode const& node_) const -> size_t
{
	return node_.m_begin.line;
}

//////////////////////////////////////////
auto Instance::evaluateType(rigc::ParserNode const& typeNode_) -> DeclType
{
	DeclType evaluatedType;
	// Note:
	// "Type" node might be either disambiguated ("name::<Params>") or not ("name<Params>")
	// Sometimes not disambiguated notation is acceptable.
	auto& actualTypeNode = (typeNode_.is_type<rigc::Type>() ? *typeNode_.children.front() : typeNode_);

	auto typeName		= findElem<rigc::Name>(actualTypeNode)->string_view();
	auto templateParams = findElem<rigc::TemplateParams>(actualTypeNode);

	if (templateParams)
	{
		if (typeName == "Ref")
		{
			auto inner = findElem<rigc::TemplateParam>(*templateParams);
			return constructTemplateType<RefType>(
					this->universalScope(),
					this->evaluateType(*findElem<rigc::Type>(*inner))
				);
		}
		else if (typeName == "Addr")
		{
			auto inner = findElem<rigc::TemplateParam>(*templateParams);
			return constructTemplateType<AddrType>(
					this->universalScope(),
					this->evaluateType(*findElem<rigc::Type>(*inner))
				);
		}
		else if (typeName == "StaticArray")
		{
			// ensure 2 template params
			if (templateParams->children.size() != 2)
				throw std::runtime_error("StaticArray requires 2 template params: StaticArray<T, N: Int32>");

			auto inner	= this->evaluateType(*findElem<rigc::Type>(*templateParams->children[0]));
			// TEMP:
			auto size	= std::stoi(templateParams->children[1]->string());

			return constructTemplateType<ArrayType>(this->universalScope(), inner, size);
		}
	}

	return this->findType(typeName)->shared_from_this();
}


//////////////////////////////////////////
auto Instance::findType(std::string_view name_) -> IType const*
{
	auto scope = currentScope;
	while (scope)
	{
		if (auto type = scope->types.find(name_))
			return type.get();

		if (auto type = scope->findType(name_))
			return type;

		scope = scope->parent;
	}

	return nullptr;
}

//////////////////////////////////////////
auto Instance::findFunction(std::string_view name_) -> FunctionCandidates
{
	auto candidates = FunctionCandidates{};
	candidates.reserve(10);
	for (auto it = stack.frames.rbegin(); it != stack.frames.rend(); ++it)
	{
		auto overloads = it->scope->findFunction(name_);
		if (overloads)
			candidates.push_back( { it->scope, overloads } );

		if (auto type = it->scope->types.find(name_))
		{
			if (auto classType = type->as<ClassType>())
			{
				if (auto ctors = classType->constructors())
					candidates.push_back( { it->scope, ctors } );
			}
		}

	}

	return candidates;
}

//////////////////////////////////////////
auto Instance::cloneValue(Value value_) -> Value
{
	return this->allocateOnStack( value_.getType(), reinterpret_cast<void*>(value_.blob()), value_.getType()->size() );
}

//////////////////////////////////////////
auto Instance::reserveOnStack(DeclType type_, bool lookBack_) -> FrameBasedValue
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
auto Instance::allocateOnStack(DeclType type_, void const* sourceBytes_, size_t toCopy) -> Value
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
auto Instance::scopeOf(void const *addr_) -> Scope&
{
	auto it = scopes.find(addr_);
	if (it == scopes.end())
	{
		auto scope = std::make_unique<Scope>(*this);
		auto& scopeRef = *scope;
		scopes[addr_] = std::move(scope);
		return scopeRef;
	}

	auto& s = *it->second;
	return s;
}

//////////////////////////////////////////
auto Instance::pushStackFrameOf(void const* addr_) -> Scope&
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
auto Instance::popStackFrame() -> void
{
	assert(stack.frames.size() > 1 && "Tried to pop a universe scope-related stack frame.");

	stack.popFrame();
	currentScope = stack.frames.back().scope;
}


}
