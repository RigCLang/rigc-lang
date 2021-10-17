#pragma once

#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Value.hpp>
#include <RigCInterpreter/Type.hpp>
#include <RigCInterpreter/Functions.hpp>

namespace rigc::vm
{

template <typename T, size_t MaxLength>
struct StaticString
	 : std::array<T, MaxLength>
{
	size_t numChars = 0;

	template <size_t OtherLen>
	constexpr StaticString& operator+=(StaticString<T, OtherLen> const& other_) {
		size_t toAdd = std::min(OtherLen, MaxLength - numChars);

		for(size_t i = 0; i < toAdd; ++i)
			(*this)[numChars + i] = other_[i];
		numChars += toAdd;

		return *this;
	}

	template <size_t OtherLen>
	constexpr StaticString& operator+=(T const (&other_)[OtherLen]) {
		size_t actualLength = OtherLen - 1;
		size_t toAdd = std::min(actualLength, MaxLength - numChars);

		for(size_t i = 0; i < toAdd; ++i)
			(*this)[numChars + i] = other_[i];
		numChars += toAdd;

		return *this;
	}

	constexpr StaticString& operator+=(std::string_view const& other_) {
		size_t toAdd = std::min(other_.length(), MaxLength - numChars);

		for(size_t i = 0; i < toAdd; ++i)
			(*this)[numChars + i] = other_[i];
		numChars += toAdd;

		return *this;
	}
};

using FunctionOverloads		= std::vector<Function*>;
using FunctionParamTypes	= std::array<DeclType, Function::MAX_PARAMS>;

Function const* findOverload(
		FunctionOverloads const&	funcs_,
		FunctionParamTypes const&	paramTypes_,
		size_t						numArgs_
	);

struct Scope
{
	using Impls				= std::vector<TypeImpl*>;

	size_t initialStackSize = 0;

	std::map<TypeBase*, Impls*>						impls;
	std::map<std::string, FunctionOverloads>		functions;
	std::map<std::string, Value>					variables;
	std::map<std::string, TypeBase const*>			typeAliases;

	static StaticString<char, 512> formatOperatorName(std::string_view opName_, Operator::Type type_)
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

	FunctionOverloads const* findFunction(std::string_view funcName_) const
	{
		auto it = functions.find(std::string(funcName_));
		if (it != functions.end())
			return &it->second;
		
		return nullptr;
	}

	TypeBase const* findType(std::string_view typeName_) const
	{
		auto it = typeAliases.find(std::string(typeName_));
		if (it != typeAliases.end())
			return it->second;
		
		return nullptr;
	}

	FunctionOverloads const* findOperator(std::string_view opName_, Operator::Type type_) const
	{
		auto fmtName = formatOperatorName(opName_, type_);
		return this->findFunction(
				std::string_view( fmtName.data(), fmtName.numChars )
			);
	}

	TypeBase& registerType(struct Instance& vm_, std::string_view name_, TypeBase type_);
	Function& registerFunction(struct Instance& vm_, std::string_view name_, Function func_);
	Function& registerOperator(struct Instance& vm_, std::string_view name_, Operator::Type type_, Function func_);
};

std::unique_ptr<Scope> makeUniverseScope(Instance &vm_);

}