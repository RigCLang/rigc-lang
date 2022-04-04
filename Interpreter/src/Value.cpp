#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Value.hpp>

#include <RigCInterpreter/TypeSystem/ClassType.hpp>
#include <RigCInterpreter/VM.hpp>

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

/////////////////////////////////////
std::string dump(Instance const& vm_, Value const& value_)
{
	auto offset = reinterpret_cast<char const*>(value_.data) - vm_.stack.container.data();

	auto& type = *value_.type.get();
	if (auto deref = type.is<RefType>())
	{
		return fmt::format("ref to ({}) => {}", offset, dump(vm_, value_.deref()));
	}
	else
	{
		auto addr = fmt::format("(addr: {}) ", offset);

		if (type.name() == "Int32")
			return addr + fmt::format("{}", value_.view<int32_t>());
		else if (type.name() == "Float32")
			return addr + fmt::format("{:.4f}", value_.view<float>());
		else if (type.name() == "Float64")
			return addr + fmt::format("{:.4f}", value_.view<double>());
		else if (type.name() == "Bool")
			return addr + fmt::format("{}", value_.view<bool>() ? "true" : "false");
		else if (auto cl = type.as<ClassType>())
		{
			std::string str;
			str.reserve(1024);
			str += "{\n";
			for (auto& dm : cl->dataMembers)
			{
				str += fmt::format("    \"{}\": {}\n", dm.name, dump(vm_, value_.member(dm)));
			}
			str += "}";
			return addr + str;
		}
	}
	return "<unknown>";
}

}
