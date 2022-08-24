#pragma once

#include RIGCVM_PCH

#include <RigCVM/TypeSystem/TypeConstraint.hpp>

namespace rigc::vm
{
////////////////////////////////////////
auto getTemplateParamList(rigc::ParserNode const& expr_) -> DynArray<Pair<std::string, TypeConstraint>>;
}
