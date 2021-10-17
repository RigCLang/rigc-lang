#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Value.hpp>
#include <RigCInterpreter/Type.hpp>
#include <RigCInterpreter/Scope.hpp>
#include <RigCInterpreter/VM.hpp>

namespace rigc::vm
{

//////////////////////////////////////
template <typename T>
OptValue builtinAddOperator(Instance &vm_, Value const& lhs_, Value const& rhs_)
{
	T const& lhsData = *reinterpret_cast<T const*>(lhs_.blob());
	T const& rhsData = *reinterpret_cast<T const*>(rhs_.blob());

	return vm_.allocateOnStack<T>(lhs_.getType(), lhsData + rhsData);
}

//////////////////////////////////////
template <typename T>
OptValue builtinSubOperator(Instance &vm_, Value const& lhs_, Value const& rhs_)
{
	T const& lhsData = *reinterpret_cast<T const*>(lhs_.blob());
	T const& rhsData = *reinterpret_cast<T const*>(rhs_.blob());

	return vm_.allocateOnStack<T>(lhs_.getType(), lhsData - rhsData);
}

//////////////////////////////////////
template <typename T>
OptValue builtinMultOperator(Instance &vm_, Value const& lhs_, Value const& rhs_)
{
	T const& lhsData = *reinterpret_cast<T const*>(lhs_.blob());
	T const& rhsData = *reinterpret_cast<T const*>(rhs_.blob());

	return vm_.allocateOnStack<T>(lhs_.getType(), lhsData * rhsData);
}

//////////////////////////////////////
template <typename T>
OptValue builtinDivOperator(Instance &vm_, Value const& lhs_, Value const& rhs_)
{
	T const& lhsData = *reinterpret_cast<T const*>(lhs_.blob());
	T const& rhsData = *reinterpret_cast<T const*>(rhs_.blob());

	return vm_.allocateOnStack<T>(lhs_.getType(), lhsData / rhsData);
}

//////////////////////////////////////
template <typename T>
OptValue builtinLowerThanOperator(Instance &vm_, Value const& lhs_, Value const& rhs_)
{
	T const& lhsData = *reinterpret_cast<T const*>(lhs_.blob());
	T const& rhsData = *reinterpret_cast<T const*>(rhs_.blob());

	return vm_.allocateOnStack<bool>("Bool", lhsData < rhsData);
}

//////////////////////////////////////
template <typename T>
OptValue builtinGreaterThanOperator(Instance &vm_, Value const& lhs_, Value const& rhs_)
{
	T const& lhsData = *reinterpret_cast<T const*>(lhs_.blob());
	T const& rhsData = *reinterpret_cast<T const*>(rhs_.blob());

	return vm_.allocateOnStack<bool>("Bool", lhsData > rhsData);
}

//////////////////////////////////////
template <typename T>
OptValue builtinLowerEqThanOperator(Instance &vm_, Value const& lhs_, Value const& rhs_)
{
	T const& lhsData = *reinterpret_cast<T const*>(lhs_.blob());
	T const& rhsData = *reinterpret_cast<T const*>(rhs_.blob());

	return vm_.allocateOnStack<bool>("Bool", lhsData <= rhsData);
}

//////////////////////////////////////
template <typename T>
OptValue builtinGreaterEqThanOperator(Instance &vm_, Value const& lhs_, Value const& rhs_)
{
	T const& lhsData = *reinterpret_cast<T const*>(lhs_.blob());
	T const& rhsData = *reinterpret_cast<T const*>(rhs_.blob());

	return vm_.allocateOnStack<bool>("Bool", lhsData >= rhsData);
}

//////////////////////////////////////
template <typename T>
OptValue builtinEqualOperator(Instance &vm_, Value const& lhs_, Value const& rhs_)
{
	T const& lhsData = *reinterpret_cast<T const*>(lhs_.blob());
	T const& rhsData = *reinterpret_cast<T const*>(rhs_.blob());

	return vm_.allocateOnStack<bool>("Bool", lhsData == rhsData);
}

//////////////////////////////////////
template <typename T>
OptValue builtinNotEqualOperator(Instance &vm_, Value const& lhs_, Value const& rhs_)
{
	T const& lhsData = *reinterpret_cast<T const*>(lhs_.blob());
	T const& rhsData = *reinterpret_cast<T const*>(rhs_.blob());

	return vm_.allocateOnStack<bool>("Bool", lhsData != rhsData);
}

//////////////////////////////////////
template <typename T>
OptValue builtinAssignOperator(Instance &vm_, Value const& lhs_, Value const& rhs_)
{
	T & lhsData = *reinterpret_cast<T *>(lhs_.blob());
	T const& rhsData = *reinterpret_cast<T const*>(rhs_.blob());

	lhsData = rhsData;

	return lhs_;
}

//////////////////////////////////////
template <typename T>
TypeBase& TypeBase::Builtin(Instance &vm_, Scope& universeScope_, std::string_view name_, size_t size_)
{
	TypeBase& t = universeScope_.registerType(vm_, name_, TypeBase(name_, size_) );

	Function::Params infixParams;
	infixParams[0] = { "lhs", DeclType::fromType(t) };
	infixParams[1] = { "rhs", DeclType::fromType(t) };

	#define MAKE_INFIX_OP(Name, Incantation) \
		static auto const& OPERATOR_##Name = [](Instance &vm_, Function::Args args_, size_t argCount_) \
			{ \
				return builtin##Name##Operator<T>(vm_, args_[0], args_[1]); \
			}; \
		universeScope_.registerOperator(vm_, Incantation, Operator::Infix, Function(OPERATOR_##Name, infixParams, 2));

	if constexpr (!std::is_same_v<T, bool>)
	{
		MAKE_INFIX_OP(Add,		"+");
		MAKE_INFIX_OP(Sub,		"-");
		MAKE_INFIX_OP(Mult,		"*");
		MAKE_INFIX_OP(Div,		"/");	

		MAKE_INFIX_OP(LowerThan,		"<");
		MAKE_INFIX_OP(GreaterThan,		">");
		MAKE_INFIX_OP(LowerEqThan,		"<=");
		MAKE_INFIX_OP(GreaterEqThan,	">=");	
	}

	MAKE_INFIX_OP(Equal,	"==");	
	MAKE_INFIX_OP(NotEqual,	"!=");	
	MAKE_INFIX_OP(Assign,	"=");	

	return t;

	#undef MAKE_INFIX_OP
}

#define LINK_BUILTIN_TYPE(TypeName) \
	template TypeBase& TypeBase::Builtin<TypeName>(Instance&, Scope&, std::string_view, size_t)

LINK_BUILTIN_TYPE(bool);
LINK_BUILTIN_TYPE(char);
LINK_BUILTIN_TYPE(char16_t);
LINK_BUILTIN_TYPE(char32_t);
LINK_BUILTIN_TYPE(int8_t);
LINK_BUILTIN_TYPE(int16_t);
LINK_BUILTIN_TYPE(int32_t);
LINK_BUILTIN_TYPE(int64_t);
LINK_BUILTIN_TYPE(uint8_t);
LINK_BUILTIN_TYPE(uint16_t);
LINK_BUILTIN_TYPE(uint32_t);
LINK_BUILTIN_TYPE(uint64_t);
LINK_BUILTIN_TYPE(float);
LINK_BUILTIN_TYPE(double);

#undef LINK_BUILTIN_TYPE

//////////////////////////////////////////////////
bool operator==(UnitDeclType const& lhs_, UnitDeclType const& rhs_)
{
	return lhs_.type == rhs_.type && lhs_.isConst == rhs_.isConst;
}

//////////////////////////////////////////////////
bool operator==(ArrayDeclType const& lhs_, ArrayDeclType const& rhs_)
{
	if(lhs_.elementType != rhs_.elementType || lhs_.isConst != rhs_.isConst)
		return false;

	for (size_t i = 0; i < ArrayDeclType::MAX_DIMENSIONS; ++i)
	{
		if (lhs_.span[i] != rhs_.span[i])
			return false;
	}

	return true;
}

//////////////////////////////////////////////////
bool operator==(DeclType const& lhs_, DeclType const& rhs_)
{
	if (&lhs_ == &rhs_)
		return true;

	if (lhs_.index() != rhs_.index())
		return false;

	if (lhs_.isArray())
		return lhs_.as<ArrayDeclType>() == rhs_.as<ArrayDeclType>();

	return lhs_.as<UnitDeclType>() == rhs_.as<UnitDeclType>();
}

}