#include RIGCVM_PCH

#include <RigCVM/Scope.hpp>
#include <RigCVM/VM.hpp>

#include <RigCVM/TypeSystem/RefType.hpp>
#include <RigCVM/TypeSystem/FuncType.hpp>

#include <RigCVM/Builtin/Functions.hpp>

namespace rigc::vm
{

////////////////////////////////////
std::unique_ptr<Scope> makeUniverseScope(Instance &vm_)
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
StaticString<char, 512> Scope::formatOperatorName(std::string_view opName_, Operator::Type type_)
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
Function const* Scope::findConversion(DeclType const& from_, DeclType const& to_) const
{
	auto overloads = this->findFunction("operator convert");
	if (!overloads)
		return nullptr;

	return findOverload(*overloads, { from_ }, 1, false, to_);
}

// TODO: add support for second-pass functions
///////////////////////////////////////////////////////////////
bool testFunctionOverload(Function& func_, FunctionParamTypes const& paramTypes_, size_t numArgs_)
{
	if (func_.paramCount != numArgs_)
		return false;

	for(size_t i = 0; i < numArgs_; ++i)
	{
		if (func_.params[i].type != paramTypes_[i])
		{
			if (auto ref = paramTypes_[i]->as<RefType>())
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
FunctionOverloads const* Scope::findFunction(std::string_view funcName_) const
{
	auto it = functions.find(funcName_);
	if (it != functions.end())
		return &it->second;

	return nullptr;
}

///////////////////////////////////////////////////////////////
Function const* findOverload(
		FunctionOverloads const&	funcs_,
		FunctionParamTypes const&	paramTypes_, size_t numArgs_,
		bool						method,
		Function::ReturnType		returnType_
	)
{
	if (funcs_.size() == 1 && funcs_[0]->variadic && funcs_[0]->isRaw())
	{
		return funcs_[0];
	}

	for (size_t i = 0; i < funcs_.size(); ++i)
	{
		if (method && funcs_[i]->params[0].name != "self")
			continue;

		if (testFunctionOverload(*funcs_[i], paramTypes_, numArgs_))
		{
			if (!returnType_ || funcs_[i]->returnType == returnType_)
				return funcs_[i];
		}
	}

	return nullptr;
}


///////////////////////////////////////////////////////////////
IType const* Scope::findType(std::string_view typeName_) const
{
	auto it = typeAliases.find(typeName_);
	if (it != typeAliases.end())
		return it->second;

	return nullptr;
}

///////////////////////////////////////////////////////////////
FunctionOverloads const* Scope::findOperator(std::string_view opName_, Operator::Type type_) const
{
	auto fmtName = formatOperatorName(opName_, type_);
	return this->findFunction(
			std::string_view( fmtName.data(), fmtName.numChars )
		);
}


///////////////////////////////////////////////////////////////
IType& Scope::registerType(Instance& vm_, std::string_view name_, IType& type_)
{
	typeAliases[std::string(name_)] = &type_;

	return type_;
}


///////////////////////////////////////////////////////////////
void Scope::addType(DeclType type_)
{
	types.add(type_);
	type_->postInitialize(*vm);
}

///////////////////////////////////////////////////////////////
Function& Scope::registerFunction(Instance& vm_, std::string_view name_, Function func_)
{
	functionStorage.emplace_back( std::make_unique<Function>(std::move(func_)) );
	Function& f = *functionStorage.back();

	// TODO: ensure unique overload signature
	FunctionOverloads& overloads = functions[ std::string(name_) ];
	overloads.emplace_back( &f );

	return f;
}

///////////////////////////////////////////////////////////////
Function& Scope::registerOperator(Instance& vm_, std::string_view name_, Operator::Type type_, Function func_)
{
	auto fmtName = formatOperatorName(name_, type_);
	return this->registerFunction(
			vm_,
			std::string_view( fmtName.data(), fmtName.numChars ),
			std::move(func_)
		);
}


}
