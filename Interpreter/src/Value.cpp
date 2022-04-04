#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Value.hpp>

#include <RigCInterpreter/TypeSystem/ClassType.hpp>

namespace rigc::vm
{

/////////////////////////////////////
Value Value::member(DataMember const& dm_) const
{
	return this->member(dm_.offset, dm_.type);
}

/////////////////////////////////////
Value Value::member(size_t offset_, DeclType type_) const
{
	Value mem;
	mem.type = std::move(type_);
	mem.data = reinterpret_cast<char*>(data) + offset_;
	return mem;
}

/////////////////////////////////////
Value Value::deref() const
{
	if (auto ref = dynamic_cast<RefType*>(type.get()))
	{
		Value val;
		val.type = ref->inner;
		val.data = this->view<void*>();
		return val;
	}
	throw std::runtime_error("Cannot deref non-ref type");
}

}
