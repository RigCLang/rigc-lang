#include RIGCVM_PCH

#include <RigCVM/TypeSystem/IType.hpp>

#include <RigCVM/TypeSystem/ClassType.hpp>

namespace rigc::vm
{

//////////////////////////////////////
void IType::addMethod(std::string_view name_, Function* func_)
{
	this->methods[name_].push_back(func_);

	func_->outerType = this;
}

}
