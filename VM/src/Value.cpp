#include RIGCVM_PCH

#include <RigCVM/Value.hpp>

#include <RigCVM/VM.hpp>
#include <RigCVM/TypeSystem/ClassType.hpp>
#include <RigCVM/TypeSystem/RefType.hpp>
#include <RigCVM/TypeSystem/FuncType.hpp>

#include <RigCVM/ErrorHandling/Exceptions.hpp>

namespace rigc::vm
{
/////////////////////////////////////
auto Value::member(DataMember const& dm_) const -> Value
{
	return this->member(dm_.offset, dm_.type);
}

/////////////////////////////////////
auto Value::member(size_t offset_, DeclType type_) const -> Value
{
	return Value {
		std::move(type_),
		reinterpret_cast<char*>(data) + offset_
	};
}

/////////////////////////////////////
auto Value::safeRemoveRef() const -> Value
{
	auto ref = type->as<RefType>();
	if(!ref)
		return *this;
	else
		return Value{
			ref->inner(),
			this->view<void*>()
		};
}

/////////////////////////////////////
auto Value::safeRemovePtr() const -> Value
{
	auto ptr = type->as<AddrType>();

	if(!ptr) 
		return *this;
	else
		return Value{
			ptr->inner(),
			this->view<void*>()
		};
}

/////////////////////////////////////
auto Value::removeRef() const -> Value
{
	auto ref = type->as<RefType>();
	assert(ref && "Cannot deref a non-ref type.");

	return Value{
		ref->inner(),
		this->view<void*>()
	};
}

/////////////////////////////////////
auto Value::removePtr() const -> Value
{
	auto ptr = type->as<AddrType>();
	assert(ptr && "Cannot deref a non-ref type.");

	return Value{
		ptr->inner(),
		this->view<void*>()
	};
}

/////////////////////////////////////
auto dump(Instance& vm_, Value const& value_) -> std::string
{
	auto offset = reinterpret_cast<char const*>(value_.data) - vm_.stack.container.data();

	auto& type = *value_.type.get();
	if (type.is<RefType>())
	{
		return fmt::format("ref to ({}) => {}", offset, dump(vm_, value_.removeRef()));
	}
	else if (type.is<AddrType>())
	{
		return fmt::format("addr of ({}) => {}", offset, dump(vm_, value_.removePtr()));
	}
	else if (type.is<FuncType>())
	{
		return fmt::format("{} => {}", type.name(), value_.data);
	}
	else if (type.is<MethodType>())
	{
		return fmt::format("{} => {}", type.name(), value_.data);
	}
	else
	{
		auto addr = fmt::format("(addr: {}) ", offset);

		if (type.name() == "Int32")
			return addr + fmt::format("{}", value_.view<int32_t>());
		if (type.name() == "Char")
			return addr + fmt::format("{}", value_.view<char>());
		else if (type.name() == "Float32")
			return addr + fmt::format("{:.4f}", value_.view<float>());
		else if (type.name() == "Float64")
			return addr + fmt::format("{:.4f}", value_.view<double>());
		else if (type.name() == "Bool")
			return addr + fmt::format("{}", value_.view<bool>() ? "true" : "false");
		else if (type.name() == "Func")
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
