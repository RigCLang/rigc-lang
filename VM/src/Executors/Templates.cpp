#include RIGCVM_PCH

#include <RigCVM/VM.hpp>
#include <RigCVM/TypeSystem/TypeConstraint.hpp>

namespace rigc::vm 
{

////////////////////////////////////////
auto getTemplateParamList(rigc::ParserNode const& expr_) -> Vec<Pair<std::string, TypeConstraint>>
{
	auto const templateParamList = findElem<rigc::TemplateDefParamList>(expr_);
	if(not templateParamList) return {};

	auto list = Vec<Pair<std::string, TypeConstraint>>();

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