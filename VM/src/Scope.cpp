#include RIGCVM_PCH

#include <RigCVM/Scope.hpp>
#include <RigCVM/VM.hpp>

#include <RigCVM/Functions.hpp>
#include <RigCVM/TypeSystem/RefType.hpp>
#include <RigCVM/TypeSystem/FuncType.hpp>

#include <RigCVM/Builtin/Functions.hpp>

namespace rigc::vm
{

////////////////////////////////////
auto makeUniverseScope(Instance &vm_) -> std::unique_ptr<Scope>
{
#define MAKE_BUILTIN_TYPE(CppName, RigCName) \
	CreateCoreType<CppName>(vm_, *scope, #RigCName)

	auto scope = std::make_unique<Scope>(vm_);

	MAKE_BUILTIN_TYPE(bool,		Bool);
	MAKE_BUILTIN_TYPE(char,		Char);
	MAKE_BUILTIN_TYPE(char16_t,	Char16);
	MAKE_BUILTIN_TYPE(char32_t,	Char32);
	MAKE_BUILTIN_TYPE(int16_t,	Int16);
	MAKE_BUILTIN_TYPE(int32_t,	Int32);
	MAKE_BUILTIN_TYPE(int64_t,	Int64);
	MAKE_BUILTIN_TYPE(uint16_t,	Uint16);
	MAKE_BUILTIN_TYPE(uint32_t,	Uint32);
	MAKE_BUILTIN_TYPE(uint64_t,	Uint64);
	MAKE_BUILTIN_TYPE(float,	Float32);
	MAKE_BUILTIN_TYPE(double,	Float64);

	scope->addType(std::make_unique<FuncType>());
	scope->addType(std::make_unique<MethodType>());

	// "print" builtin function
	{
		auto func = Function{ &builtin::print, {}, 0 };
		func.variadic = true;

		scope->registerFunction(vm_, "print", std::move(func));
	}
	// "typeof" builtin function
	{
		auto func = Function{ &builtin::typeOf, {}, 0 };
		func.variadic = true;

		scope->registerFunction(vm_, "typeOf", std::move(func));
	}
	// "readInt" builtin function
	{
		auto func = Function{ &builtin::readInt, {}, 0 };
		func.variadic = true;

		scope->registerFunction(vm_, "readInt", std::move(func));
	}
	// "readFloat" builtin function
	{
		auto func = Function{ &builtin::readFloat, {}, 0 };
		func.variadic = true;

		scope->registerFunction(vm_, "readFloat", std::move(func));
	}

	return scope;
#undef ADD_BUILTIN_TYPE
#undef MAKE_BUILTIN_TYPE
}

///////////////////////////////////////////////////////////////
auto Scope::formatOperatorName(std::string_view opName_, Operator::Type type_) -> StaticString<char, 512>
{
	constexpr char opPrefix[]		= "operator ";
	constexpr char prefixOpText[]	= "pr";
	constexpr char postfixOpText[]	= "po";
	constexpr char infixOpText[]	= "in";

	StaticString<char, 512> fmtName;
	fmtName += opPrefix;
	if (type_ == Operator::Infix)
		fmtName += infixOpText;
	else if (type_ == Operator::Prefix)
		fmtName += prefixOpText;
	else if (type_ == Operator::Postfix)
		fmtName += postfixOpText;

	fmtName += opName_;
	return fmtName;
}

///////////////////////////////////////////////////////////////
auto Scope::findConversion(DeclType const& from_, DeclType const& to_) const -> Function const*
{
	auto overloads = this->findFunction("operator convert");
	if (!overloads)
		return nullptr;

	auto types = FunctionParamTypes{ from_ };
	return findOverload(*overloads,  { types.data(), 1 }, false, to_);
}

// TODO: add support for second-pass functions
///////////////////////////////////////////////////////////////
auto testFunctionOverload(Function& func_, FunctionParamTypeSpan paramTypes_) -> bool
{
	auto visibleParamCount = func_.paramCount;

	// "self" param is not visible in constructors
	if (func_.isConstructor)
		visibleParamCount -= 1;

	if (visibleParamCount != paramTypes_.size())
		return false;

	// Ignore the `self` parameter for constructors
	size_t i = func_.isConstructor ? 1 : 0;
	size_t testedIdx = 0;
	for(; i < paramTypes_.size(); ++i, ++testedIdx)
	{
		if (func_.params[i].type != paramTypes_[testedIdx])
		{
			if (auto ref = paramTypes_[testedIdx]->as<RefType>())
			{
				if (ref->inner.get() != func_.params[i].type.get())
					return false;
			}
			else
				return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////
auto Scope::findFunction(std::string_view funcName_) const -> FunctionOverloads const*
{
	auto it = functions.find(funcName_);
	if (it != functions.end())
		return &it->second;

	return nullptr;
}

///////////////////////////////////////////////////////////////
auto Scope::tryGenerateFunction(
		Instance &vm_,
		std::string_view			funcName_,
		FunctionParamTypeSpan		paramTypes_
	) -> Function const*
{
	auto templIt = functionTemplates.find(funcName_);
	if (templIt == functionTemplates.end())
	{
		if (parent)
			return parent->tryGenerateFunction(vm_, funcName_, paramTypes_);
		return nullptr;
	}

	for (auto& templ : templIt->second)
	{
		if (paramTypes_.size() != templ->paramCount)
			continue;

		// if (testFunctionOverload(*templ, paramTypes_))
		{
			auto params = Function::Params();
			for (size_t i = 0; i < paramTypes_.size(); ++i)
			{
				if (!templ->params[i].type)
					params[i] = { templ->params[i].name, paramTypes_[i] }; // TODO: evaluate constraint
				else
					params[i] = { templ->params[i].name, templ->params[i].type };
			}

			auto paramsString = std::string();
			for (size_t i = 0; i < paramTypes_.size(); ++i)
			{
				if (i > 0)
					paramsString += ", ";
				paramsString += params[i].type->name();
			}

			// fmt::print("Instantiating function template {} with params: {}\n", funcName_, paramsString);

			return &this->registerFunction(vm_, funcName_,
					Function(
						Function::RuntimeFn(templ->runtimeImpl().node),
						std::move(params),
						paramTypes_.size()
					)
				);
		}
	}

	if (parent)
		return parent->tryGenerateFunction(vm_, funcName_, paramTypes_);
	return nullptr;
}

///////////////////////////////////////////////////////////////
auto findOverload(
		FunctionCandidates const&	funcs_,
		FunctionParamTypeSpan		paramTypes_,
		bool						method_,
		Function::ReturnType		returnType_
	) -> Function const*
{
	for (auto& [scope, overloads] : funcs_)
	{
		auto result = findOverload(*overloads, paramTypes_, method_, returnType_);
		if (result)
			return result;
	}

	return nullptr;
}

///////////////////////////////////////////////////////////////
auto findOverload(
		FunctionOverloads const&	overloads_,
		FunctionParamTypeSpan		paramTypes_,
		bool						method_,
		Function::ReturnType		returnType_
	) -> Function const*
{
	if (overloads_.size() == 1 && overloads_[0]->variadic && overloads_[0]->isRaw())
	{
		return overloads_[0];
	}

	for (size_t i = 0; i < overloads_.size(); ++i)
	{
		if (method_ && overloads_[i]->params[0].name != "self")
			continue;

		if (testFunctionOverload(*overloads_[i], paramTypes_))
		{
			if (!returnType_ || overloads_[i]->returnType == returnType_)
				return overloads_[i];
		}
	}

	return nullptr;
}


///////////////////////////////////////////////////////////////
auto Scope::findType(std::string_view typeName_) const -> IType const*
{
	auto it = typeAliases.find(typeName_);
	if (it != typeAliases.end())
		return it->second;

	// Find within function instantiation
	if (func)
	{
		auto& impl = func->runtimeImpl();
		if (impl.templateArguments)
		{
			auto it = impl.templateArguments->find(typeName_);
			if (it != impl.templateArguments->end())
			{
				auto& arg = it->second;
				if (arg.is<DeclType>())
					return arg.as<DeclType>().get();
			}
		}
	}

	return nullptr;
}

///////////////////////////////////////////////////////////////
auto Scope::findOperator(std::string_view opName_, Operator::Type type_) const -> FunctionOverloads const*
{
	auto fmtName = formatOperatorName(opName_, type_);
	return this->findFunction(
			std::string_view( fmtName.data(), fmtName.numChars )
		);
}


///////////////////////////////////////////////////////////////
auto Scope::registerType(Instance& vm_, std::string_view name_, IType& type_) -> IType&
{
	typeAliases[std::string(name_)] = &type_;

	return type_;
}


///////////////////////////////////////////////////////////////
auto Scope::addType(MutDeclType type_) -> void
{
	types.add(type_);
	type_->postInitialize(*vm);
}

///////////////////////////////////////////////////////////////
auto Scope::registerFunction(Instance& vm_, std::string_view name_, Function func_) -> Function&
{
	functionStorage.emplace_back( std::make_unique<Function>(std::move(func_)) );
	Function& f = *functionStorage.back();

	// TODO: ensure unique overload signature
	FunctionOverloads& overloads = functions[ std::string(name_) ];
	overloads.emplace_back( &f );

	return f;
}

///////////////////////////////////////////////////////////////
auto Scope::registerFunctionTemplate(Instance& vm_, std::string_view name_, Function func_) -> Function&
{
	functionStorage.emplace_back( std::make_unique<Function>(std::move(func_)) );
	Function& f = *functionStorage.back();

	// TODO: ensure unique overload signature
	FunctionOverloads& overloads = functionTemplates[ std::string(name_) ];
	overloads.emplace_back( &f );

	return f;
}

///////////////////////////////////////////////////////////////
auto Scope::registerOperator(Instance& vm_, std::string_view name_, Operator::Type type_, Function func_) -> Function&
{
	auto fmtName = formatOperatorName(name_, type_);
	return this->registerFunction(
			vm_,
			std::string_view( fmtName.data(), fmtName.numChars ),
			std::move(func_)
		);
}


}
