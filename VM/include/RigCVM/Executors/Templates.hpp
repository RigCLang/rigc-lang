#pragma once

#include <RigCVM/RigCVMPCH.hpp>

#include <RigCVM/TypeSystem/TypeConstraint.hpp>

namespace rigc::vm
{
////////////////////////////////////////
auto getTemplateParamList(rigc::ParserNode const& expr_) -> DynArray<Pair<String, TypeConstraint>>;
}
