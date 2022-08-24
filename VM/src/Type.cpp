#include RIGCVM_PCH

#include <RigCVM/Value.hpp>
#include <RigCVM/Type.hpp>
#include <RigCVM/Scope.hpp>
#include <RigCVM/VM.hpp>

#include <RigCVM/TypeSystem/RefType.hpp>

namespace rigc::vm
{
#define DEFINE_BUILTIN_MATH_OP(Name, Symbol) \
	template <typename T> \
	auto builtin##Name##Operator(Instance &vm_, Value const& lhs_, Value const& rhs_) -> OptValue \
	{ \
		T const& lhsData = *reinterpret_cast<T const*>(lhs_.blob()); \
		T const& rhsData = *reinterpret_cast<T const*>(rhs_.blob()); \
		\
		return vm_.allocateOnStack<T>(lhs_.getType(), lhsData Symbol rhsData); \
	}

#define DEFINE_BUILTIN_RELATIONAL_OP(Name, Symbol) \
	template <typename T> \
	auto builtin##Name##Operator(Instance &vm_, Value const& lhs_, Value const& rhs_) -> OptValue \
	{ \
		T const& lhsData = *reinterpret_cast<T const*>(lhs_.blob()); \
		T const& rhsData = *reinterpret_cast<T const*>(rhs_.blob()); \
		\
		return vm_.allocateOnStack<bool>("Bool", lhsData Symbol rhsData); \
	}

#define DEFINE_BUILTIN_ASSIGN_OP(Name, Symbol)												\
	template <typename T>																	\
	auto builtin##Name##Operator(Instance &vm_, Value const& lhs_, Value const& rhs_)	-> OptValue \
	{																						\
		T&			lhsData =  lhs_.removeRef().view<T>();										\
		T const&	rhsData = *reinterpret_cast<T const*>(rhs_.blob());						\
																							\
		lhsData Symbol rhsData;																\
																							\
		return lhs_;																		\
	}

#define DEFINE_BUILTIN_POSTFIX_OP(Name, Symbol)												\
	template <typename T>																	\
	auto builtin##Name##Operator(Instance &vm_, Value const& lhs_)	-> OptValue \
	{																						\
		T&			lhsData =  lhs_.removeRef().view<T>();										\
																							\
		lhsData Symbol;																\
																							\
		return lhs_;																		\
	}

#define DEFINE_BUILTIN_PREFIX_OP(Name, Symbol)												\
	template <typename T>																	\
	auto builtin##Name##Operator(Instance &vm_, Value const& lhs_)	-> OptValue \
	{																						\
		T&			lhsData =  lhs_.removeRef().view<T>();										\
																							\
		Symbol lhsData;																\
																							\
		return lhs_;																		\
	}

DEFINE_BUILTIN_MATH_OP		(Add,	+);
DEFINE_BUILTIN_MATH_OP		(Sub,	-);
DEFINE_BUILTIN_MATH_OP		(Mult,	*);
DEFINE_BUILTIN_MATH_OP		(Div,	/);
DEFINE_BUILTIN_MATH_OP		(Mod,	%);

DEFINE_BUILTIN_RELATIONAL_OP(LowerThan,		<);
DEFINE_BUILTIN_RELATIONAL_OP(GreaterThan,	>);
DEFINE_BUILTIN_RELATIONAL_OP(LowerEqThan,	<=);
DEFINE_BUILTIN_RELATIONAL_OP(GreaterEqThan,	>=);
DEFINE_BUILTIN_RELATIONAL_OP(Equal,			==);
DEFINE_BUILTIN_RELATIONAL_OP(NotEqual,		!=);

DEFINE_BUILTIN_RELATIONAL_OP(LogicalAnd,	&&);
DEFINE_BUILTIN_RELATIONAL_OP(LogicalOr,		||);

DEFINE_BUILTIN_ASSIGN_OP	(Assign,		=);
DEFINE_BUILTIN_ASSIGN_OP	(AddAssign,		+=);
DEFINE_BUILTIN_ASSIGN_OP	(SubAssign,		-=);
DEFINE_BUILTIN_ASSIGN_OP	(MultAssign,	*=);
DEFINE_BUILTIN_ASSIGN_OP	(DivAssign,		/=);
DEFINE_BUILTIN_ASSIGN_OP	(ModAssign,		%=);

DEFINE_BUILTIN_POSTFIX_OP(PostIncrement, ++);
DEFINE_BUILTIN_POSTFIX_OP(PostDecrement, --);

DEFINE_BUILTIN_PREFIX_OP(PreIncrement, ++);
DEFINE_BUILTIN_PREFIX_OP(PreDecrement, --);

#undef DEFINE_BUILTIN_MATH_OP
#undef DEFINE_BUILTIN_RELATIONAL_OP
#undef DEFINE_BUILTIN_ASSIGN_OP
#undef DEFINE_BUILTIN_POSTFIX_OP
#undef DEFINE_BUILTIN_PREFIX_OP

//////////////////////////////////////
template <typename T>
auto CreateCoreType(Instance &vm_, Scope& universeScope_, std::string_view name_, size_t size_) -> IType*
{
	auto t = std::make_shared<CoreType>(CoreType::fromCppType<T>());
	universeScope_.addType(t);

	if constexpr (!std::is_same_v<T, void>)
	{
		Function::Params infixParams;
		infixParams[0] = { "lhs", t };
		infixParams[1] = { "rhs", t };

		auto refToType = constructTemplateType<RefType>(universeScope_, t);

		Function::Params infixAssignParams;
		infixAssignParams[0] = { "lhs", refToType };
		infixAssignParams[1] = { "rhs", t };

		Function::Params prePostfixParams;
		prePostfixParams[0] = { "lhs", refToType };

		#define MAKE_INFIX_OP(Name, Incantation) \
			static auto const& OPERATOR_##Name = [](Instance &vm_, Function::ArgSpan args_) \
				{ \
					return builtin##Name##Operator<T>(vm_, args_[0], args_[1]); \
				}; \
			{ \
				auto& op = universeScope_.registerOperator(vm_, Incantation, Operator::Infix, Function(OPERATOR_##Name, infixParams, 2)); \
				op.returnType = t; \
				op.raw().name = fmt::format("operator {} (lhs: {}, rhs: {}) -> {}", \
						Incantation, \
						infixParams[0].type->name(), infixParams[1].type->name(), \
						op.returnType->name() \
					); \
			}

		#define MAKE_INFIX_REL_OP(Name, Incantation) \
			static auto const& OPERATOR_##Name = [](Instance &vm_, Function::ArgSpan args_) \
				{ \
					return builtin##Name##Operator<T>(vm_, args_[0], args_[1]); \
				}; \
			{ \
				auto& op = universeScope_.registerOperator(vm_, Incantation, Operator::Infix, Function(OPERATOR_##Name, infixParams, 2)); \
				op.returnType = vm_.findType("Bool")->shared_from_this(); \
				op.raw().name = fmt::format("operator {} (lhs: {}, rhs: {}) -> {}", \
						Incantation, \
						infixParams[0].type->name(), infixParams[1].type->name(), \
						op.returnType->name() \
					); \
			}

		#define MAKE_INFIX_ASSIGN_OP(Name, Incantation) \
			static auto const& OPERATOR_##Name = [](Instance &vm_, Function::ArgSpan args_) \
				{ \
					return builtin##Name##Operator<T>(vm_, args_[0], args_[1]); \
				}; \
			{ \
				auto& op = universeScope_.registerOperator(vm_, Incantation, Operator::Infix, Function(OPERATOR_##Name, infixAssignParams, 2)); \
				op.returnType = refToType; \
				op.returnsRef = true; \
				op.raw().name = fmt::format("operator {} (lhs: {}, rhs: {}) -> {}", \
						Incantation, \
						infixAssignParams[0].type->name(), infixAssignParams[1].type->name(), \
						op.returnType->name() \
					); \
			}

		#define MAKE_POSTFIX_OP(Name, Incantation) \
			static auto const& OPERATOR_##Name = [](Instance &vm_, Function::ArgSpan args_) \
				{ \
					return builtin##Name##Operator<T>(vm_, args_[0]); \
				}; \
			{ \
				auto& op = universeScope_.registerOperator(vm_, Incantation, Operator::Postfix, Function(OPERATOR_##Name, prePostfixParams, 1)); \
				op.returnType = t; \
				op.raw().name = fmt::format("operator post {} (lhs: {}) -> {}", \
						Incantation, \
						prePostfixParams[0].type->name(), \
						op.returnType->name() \
					); \
			}

		#define MAKE_PREFIX_OP(Name, Incantation) \
			static auto const& OPERATOR_##Name = [](Instance &vm_, Function::ArgSpan args_) \
				{ \
					return builtin##Name##Operator<T>(vm_, args_[0]); \
				}; \
			{ \
				auto& op = universeScope_.registerOperator(vm_, Incantation, Operator::Prefix, Function(OPERATOR_##Name, prePostfixParams, 1)); \
				op.returnType = refToType; \
				op.returnsRef = true; \
				op.raw().name = fmt::format("operator pre {} (rhs: {}) -> {}", \
						Incantation, \
						prePostfixParams[0].type->name(), \
						op.returnType->name() \
					); \
			}

		if constexpr (!std::is_same_v<T, bool>)
		{
			// Math
			MAKE_INFIX_OP(Add,				"+");
			MAKE_INFIX_OP(Sub,				"-");
			MAKE_INFIX_OP(Mult,				"*");
			MAKE_INFIX_OP(Div,				"/");

			if constexpr (!std::is_floating_point_v<T>)
			{
				MAKE_INFIX_OP(Mod,			"%");
				MAKE_INFIX_ASSIGN_OP(ModAssign,	"%=");
			}

			// Math (assignment)
			MAKE_INFIX_ASSIGN_OP(AddAssign,		"+=");
			MAKE_INFIX_ASSIGN_OP(SubAssign,		"-=");
			MAKE_INFIX_ASSIGN_OP(MultAssign,	"*=");
			MAKE_INFIX_ASSIGN_OP(DivAssign,		"/=");

			// Postfix
			MAKE_POSTFIX_OP(PostIncrement, "++");
			MAKE_POSTFIX_OP(PostDecrement, "--");

			// Prefix
			MAKE_PREFIX_OP(PreIncrement, "++");
			MAKE_PREFIX_OP(PreDecrement, "--");

			// Relational
			MAKE_INFIX_REL_OP(LowerThan,		"<");
			MAKE_INFIX_REL_OP(GreaterThan,		">");
			MAKE_INFIX_REL_OP(LowerEqThan,		"<=");
			MAKE_INFIX_REL_OP(GreaterEqThan,	">=");
		}

		if constexpr (std::is_same_v<T, bool>)
		{
			// Logical
			MAKE_INFIX_OP(LogicalAnd,	"and");
			MAKE_INFIX_OP(LogicalOr,	"or");
		}

		// Relational
		MAKE_INFIX_REL_OP(Equal,	"==");
		MAKE_INFIX_REL_OP(NotEqual,	"!=");

		// Assignment
		MAKE_INFIX_ASSIGN_OP(Assign,	"=");


		#undef MAKE_INFIX_OP
		#undef MAKE_INFIX_ASSIGN_OP
		#undef MAKE_POSTFIX_OP
		#undef MAKE_PREFIX_OP
	}

	return t.get();
}

template <typename T>
auto addTypeConversion(Instance &vm_, Scope& universeScope_, DeclType const& from_, DeclType const& to_, ConversionFunc& func_) -> void
{
	Function::Params convertParams;
	convertParams[0] = { "from", from_ };

	auto const& OPERATOR_Convert = [&func_](Instance &vm_, Function::ArgSpan args_)
		{
			return func_(vm_, args_[0]);
		};
	Function f(OPERATOR_Convert, convertParams, 1);
	f.returnType = to_;
	universeScope_.registerFunction( vm_, "operator convert", std::move(f) );
}

template <typename T>
auto addTypeConversion(Instance &vm_, Scope& universeScope_, std::string_view from_, std::string_view to_, ConversionFunc& func_) -> void
{
	addTypeConversion<T>(
			vm_, universeScope_,
			vm_.findType(from_)->shared_from_this(),
			vm_.findType(to_)->shared_from_this(),
			func_
		);
}

#define LINK_BUILTIN_TYPE(TypeName) \
	template IType* CreateCoreType<TypeName>(Instance&, Scope&, std::string_view, size_t); \
	template auto addTypeConversion<TypeName>(Instance&, Scope&, DeclType const&, DeclType const&, ConversionFunc&) -> void; \
	template auto addTypeConversion<TypeName>(Instance&, Scope&, std::string_view, std::string_view, ConversionFunc&) -> void


LINK_BUILTIN_TYPE(void);
LINK_BUILTIN_TYPE(bool);
LINK_BUILTIN_TYPE(char);
LINK_BUILTIN_TYPE(char16_t);
LINK_BUILTIN_TYPE(char32_t);
LINK_BUILTIN_TYPE(int16_t);
LINK_BUILTIN_TYPE(int32_t);
LINK_BUILTIN_TYPE(int64_t);
LINK_BUILTIN_TYPE(uint16_t);
LINK_BUILTIN_TYPE(uint32_t);
LINK_BUILTIN_TYPE(uint64_t);
LINK_BUILTIN_TYPE(float);
LINK_BUILTIN_TYPE(double);

#undef LINK_BUILTIN_TYPE

// //////////////////////////////////////////////////
// bool operator==(UnitDeclType const& lhs_, UnitDeclType const& rhs_)
// {
// 	return lhs_.type == rhs_.type && lhs_.isConst == rhs_.isConst;
// }

// //////////////////////////////////////////////////
// bool operator==(ArrayDeclType const& lhs_, ArrayDeclType const& rhs_)
// {
// 	if(lhs_.type != rhs_.type || lhs_.isConst != rhs_.isConst)
// 		return false;

// 	for (size_t i = 0; i < ArrayDeclType::MAX_DIMENSIONS; ++i)
// 	{
// 		if (lhs_.span[i] != rhs_.span[i])
// 			return false;
// 	}

// 	return true;
// }

// //////////////////////////////////////////////////
// bool operator==(DeclType const& lhs_, DeclType const& rhs_)
// {
// 	if (&lhs_ == &rhs_)
// 		return true;

// 	if (lhs_.index() != rhs_.index())
// 		return false;

// 	if (lhs_.isArray())
// 		return lhs_.as<ArrayDeclType>() == rhs_.as<ArrayDeclType>();

// 	return lhs_.as<UnitDeclType>() == rhs_.as<UnitDeclType>();
// }
}
