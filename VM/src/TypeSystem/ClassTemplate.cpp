#include "VM/include/RigCVM/RigCVMPCH.hpp"

#include <RigCVM/TypeSystem/ClassTemplate.hpp>

namespace rigc::vm
{

auto ClassTemplate::instantiate(vm::Instance& vm_, std::vector<TemplateParameterValue> const& params_) const -> DeclType
{
	// TODO: implement this
	return nullptr;
}
}
