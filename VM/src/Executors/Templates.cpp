#include "VM/include/RigCVM/RigCVMPCH.hpp"

#include <RigCVM/VM.hpp>
#include <RigCVM/TypeSystem/TypeConstraint.hpp>

namespace rigc::vm
{
////////////////////////////////////////
auto getTemplateParamList(rigc::ParserNode const& expr_) -> DynArray<Pair<String, TypeConstraint>>
{
	auto const templateParamList = findElem<rigc::TemplateDefParamList>(expr_);
	if(not templateParamList) return {};

	auto list = DynArray<Pair<String, TypeConstraint>>();

	auto i = 0;
	for(auto const& templateParam : templateParamList->children)
	{
		auto const paramName = findElem<rigc::Name>(*templateParam);
		auto const constraintName = findElem<rigc::TemplateDefParamKind>(*templateParam);

		list.emplace_back(paramName->string(), constraintName->string());
	}

	return list;
}
}
