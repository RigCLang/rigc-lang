#pragma once

#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/TypeSystem/IType.hpp>

namespace rigc::vm
{

struct CoreType
	: IType
{
	enum Kind : uint8_t
	{
		Int8,		Int16,		Int32,		Int64,
		Uint8,		Uint16,		Uint32,		Uint64,
		Float32,
		Float64,
		Char,		Char16,		Char32,
		Bool,
	};

	template <typename T>
	static Kind fromCppType()
	{
		#define HANDLE_TYPE(CppName, EnumValue) if constexpr (std::is_same_v<T, CppName>) return EnumValue;
		#define ELSE_HANDLE_TYPE(CppName, EnumValue) else if constexpr (std::is_same_v<T, CppName>) return EnumValue;

		HANDLE_TYPE(int8_t, 		Int8)
		ELSE_HANDLE_TYPE(int16_t,	Int16)
		ELSE_HANDLE_TYPE(int32_t,	Int32)
		ELSE_HANDLE_TYPE(int64_t,	Int64)
		ELSE_HANDLE_TYPE(uint8_t,	Uint8)
		ELSE_HANDLE_TYPE(uint16_t,	Uint16)
		ELSE_HANDLE_TYPE(uint32_t,	Uint32)
		ELSE_HANDLE_TYPE(uint64_t,	Uint64)
		ELSE_HANDLE_TYPE(float,		Float32)
		ELSE_HANDLE_TYPE(double,	Float64)
		ELSE_HANDLE_TYPE(char,		Char)
		ELSE_HANDLE_TYPE(char16_t,	Char16)
		ELSE_HANDLE_TYPE(char32_t,	Char32)
		ELSE_HANDLE_TYPE(bool,		Bool)
		else
			static_assert(std::is_same_v<T, bool>, "Unsupported type");
		#undef HANDLE_TYPE
		#undef ELSE_HANDLE_TYPE
	}

	std::string_view toString() const
	{
		switch (kind)
		{
		case Int8:		return "Int8";
		case Int16:		return "Int16";
		case Int32:		return "Int32";
		case Int64:		return "Int64";
		case Uint8:		return "Uint8";
		case Uint16:	return "Uint16";
		case Uint32:	return "Uint32";
		case Uint64:	return "Uint64";
		case Float32:	return "Float32";
		case Float64:	return "Float64";
		case Char:		return "Char";
		case Char16:	return "Char16";
		case Char32:	return "Char32";
		case Bool:		return "Bool";
		default:		return "<unknown>";
		}
	}

	std::size_t size() const override
	{
		switch (kind)
		{
		case Int16: 	return sizeof(int16_t);
		case Int32: 	return sizeof(int32_t);
		case Int64: 	return sizeof(int64_t);
		case Uint16: 	return sizeof(uint16_t);
		case Uint32: 	return sizeof(uint32_t);
		case Uint64: 	return sizeof(uint64_t);
		case Float32: 	return sizeof(float);
		case Float64: 	return sizeof(double);
		case Char: 		return sizeof(char);
		case Char16: 	return sizeof(char16_t);
		case Char32: 	return sizeof(char32_t);
		case Bool: 		return sizeof(bool);
		default:		return 0;
		}
	}

	std::string name() const override
	{
		return std::string(this->toString());
	}

	InnerType decay() const override
	{
		return const_cast<CoreType*>(this)->shared_from_this();
	}

	CoreType(Kind kind_)
		: kind(kind_)
	{}

	Kind kind;
};


}
