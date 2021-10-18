#pragma once

#include RIGCINTERPRETER_PCH

// #include <RigCInterpreter/Value.hpp>

namespace rigc::vm
{

struct Function;

struct Operator
{
	enum Type {
		Prefix,
		Infix,
		Postfix,
	};
	std::string_view	str;
	Type				type;
};

struct TypeBase
{
	TypeBase() = default;
	TypeBase(std::string_view name_, size_t size_)
		:
		name(name_),
		size(size_)
	{}
	~TypeBase() = default;

	struct Field
	{
		std::string_view	name;
		TypeBase*			type;
	};

	std::string			name;
	size_t				size;

	// std::map<Operator,			Function*>	operators;
	// std::map<std::string_view,	Function*>	methods;
	std::vector<Field>						orderedFields;
	std::map<std::string_view,	TypeBase*>	fieldsByName;

	template <typename T>
	static TypeBase& Builtin(struct Instance &vm_, struct Scope& universeScope_, std::string_view name_, size_t size_);
};

struct TypeImpl
{
	std::map<std::string_view,	Function*>	methods;
};

struct UnitDeclType
{
	TypeBase const*	type;
	bool			isConst;

	UnitDeclType nonQualified() const {
		UnitDeclType t = *this;
		t.isConst = false;
		return t;
	}
};

struct ArrayDeclType
	: UnitDeclType
{
	static constexpr size_t MAX_DIMENSIONS = 5;
	using Span = std::array<size_t, MAX_DIMENSIONS>;

	UnitDeclType	elementType;
	Span			span{0};

	size_t size() const {
		size_t elements = 1;
		for(size_t d = 0; d < MAX_DIMENSIONS && span[d] != 0; ++d)
		{
			elements *= span[d];
		}
		return (elements * elementType.type->size);
	}

	constexpr
	size_t dimensions() const {

		size_t d = 1;
		for(; d < MAX_DIMENSIONS; ++d)
		{
			if (span[d] == 0)
				return d;
		}

		return d;
	}
};


using DeclTypeVariant = std::variant<UnitDeclType, ArrayDeclType>;

struct DeclType
	: DeclTypeVariant
{
	using DeclTypeVariant::DeclTypeVariant;

	static DeclType fromType(TypeBase const& type_)
	{
		return UnitDeclType( &type_, false );
	}

	UnitDeclType& decay()
	{
		if (this->isArray())
			return this->as<ArrayDeclType>().elementType;

		return this->as<UnitDeclType>();
	}

	UnitDeclType const& decay() const
	{
		if (this->isArray())
			return this->as<ArrayDeclType>().elementType;

		return this->as<UnitDeclType>();
	}

	
	bool isConst() const {
		if (this->isArray())
			return this->as<ArrayDeclType>().isConst;

		return this->as<UnitDeclType>().isConst;
	}

	bool isArray() const {
		return this->is<ArrayDeclType>();
	}

	template <typename T>
	bool is() const {
		return std::holds_alternative<T>(*this);
	}

	template <typename T>
	T const& as() const {
		return std::get<T>(*this);
	}

	template <typename T>
	T& as() {
		return std::get<T>(*this);
	}

	size_t size() const {
		if (this->isArray())
		{
			return (this->as<ArrayDeclType>().size());
		}
		
		return this->as<UnitDeclType>().type->size;
	}
};

bool operator==(UnitDeclType const& lhs_, UnitDeclType const& rhs_);
bool operator==(ArrayDeclType const& lhs_, ArrayDeclType const& rhs_);
bool operator==(DeclType const& lhs_, DeclType const& rhs_);

}