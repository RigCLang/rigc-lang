#include RIGCVM_PCH

#include <RigCVM/TypeSystem/IType.hpp>

#include <RigCVM/TypeSystem/ClassType.hpp>

namespace rigc::vm
{

//////////////////////////////////////
auto IType::addMethod(std::string_view name_, Function* func_) -> void
{
	this->methods[name_].push_back(func_);
	func_->outerType = this;
}
}
