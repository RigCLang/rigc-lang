#pragma once

#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Value.hpp>
#include <RigCInterpreter/Type.hpp>
#include <RigCInterpreter/Functions.hpp>
#include <RigCInterpreter/InlineString.hpp>
#include <RigCInterpreter/StackFrame.hpp>

namespace rigc::vm
{

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
	size_t baseFrameOffset = 0;

	std::map<TypeBase*, Impls*>									impls;
	std::map<std::string, FunctionOverloads, std::less<> >		functions;
	std::map<std::string, FrameBasedValue, std::less<> >		variables;
	std::map<std::string, TypeBase const*, std::less<> >		typeAliases;

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
		auto it = functions.find(funcName_);
		if (it != functions.end())
			return &it->second;

		return nullptr;
	}

	TypeBase const* findType(std::string_view typeName_) const
	{
		auto it = typeAliases.find(typeName_);
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
