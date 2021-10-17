#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Scope.hpp>
#include <RigCInterpreter/VM.hpp>

namespace rigc::vm
{

////////////////////////////////////
std::unique_ptr<Scope> makeUniverseScope(Instance &vm_)
{
#define MAKE_BUILTIN_TYPE(CppName, RigCName) \
	TypeBase::Builtin<CppName>(vm_, *scope, #RigCName, sizeof(CppName))

	auto scope = std::make_unique<Scope>();

	MAKE_BUILTIN_TYPE(bool,		Bool);
	MAKE_BUILTIN_TYPE(char,		Char);
	MAKE_BUILTIN_TYPE(char16_t,	Char16);
	MAKE_BUILTIN_TYPE(char32_t,	Char32);
	MAKE_BUILTIN_TYPE(int8_t,	Int8);
	MAKE_BUILTIN_TYPE(int16_t,	Int16);
	MAKE_BUILTIN_TYPE(int32_t,	Int32);
	MAKE_BUILTIN_TYPE(int64_t,	Int64);
	MAKE_BUILTIN_TYPE(uint8_t,	Uint8);
	MAKE_BUILTIN_TYPE(uint16_t,	Uint16);
	MAKE_BUILTIN_TYPE(uint32_t,	Uint32);
	MAKE_BUILTIN_TYPE(uint64_t,	Uint64);
	MAKE_BUILTIN_TYPE(float,	Float32);
	MAKE_BUILTIN_TYPE(double,	Float64);

	return scope;
#undef ADD_BUILTIN_TYPE
#undef MAKE_BUILTIN_TYPE
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
			return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////
Function const* findOverload(FunctionOverloads const& funcs_, FunctionParamTypes const& paramTypes_, size_t numArgs_)
{
	for (size_t i = 0; i < funcs_.size(); ++i)
	{
		if (testFunctionOverload(*funcs_[i], paramTypes_, numArgs_))
			return funcs_[i];
	}

	return nullptr;
}

///////////////////////////////////////////////////////////////
TypeBase& Scope::registerType(struct Instance& vm_, std::string_view name_, TypeBase type_)
{
	TypeBase& t = vm_.registerType( std::move(type_) );

	typeAliases[std::string(name_)] = &t;

	return t;
}


///////////////////////////////////////////////////////////////
Function& Scope::registerFunction(Instance& vm_, std::string_view name_, Function func_)
{
	Function& f = vm_.registerFunction( std::move(func_) );

	// TODO: ensure unique overload signature
	FunctionOverloads& overloads = functions[ std::string(name_) ];
	overloads.emplace_back( &f );

	return f;
}

///////////////////////////////////////////////////////////////
Function& Scope::registerOperator(struct Instance& vm_, std::string_view name_, Operator::Type type_, Function func_)
{
	auto fmtName = formatOperatorName(name_, type_);
	return this->registerFunction(
			vm_,
			std::string_view( fmtName.data(), fmtName.numChars ),
			std::move(func_)
		);
}


}