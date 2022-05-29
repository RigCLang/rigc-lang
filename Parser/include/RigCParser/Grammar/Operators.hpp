#pragma once

#include RIGCPARSER_PCH

#include <RigCParser/Grammar/Characters.hpp>
#include <RigCParser/Grammar/Keywords.hpp>

namespace rigc
{

struct Expression;
struct InfixOperator;
struct PrefixOperator;
struct PostfixOperator;
struct ListOfFunctionArguments;

// Pointer and reference operators
//// Prefix
struct DereferenceOp	: p::one	<'*'> {};
struct ReferenceOp		: p::one	<'&'> {};

// Direct assignment operator
//// Infix
struct AssignOp			: p::string	<'='> {};

// Comma operator
//// Infix
struct CommaOp			: p::string	<','> {};

// Dot operator
//// Infix
struct DotOp			: p::string	<'.'> {};

// Subscript operator
//// Postfix
struct SubscriptOp		: p::seq	< p::one<'['>, OptWs, Expression, OptWs, p::one<']'> > {};
struct EmptySubscriptOp		: p::string<'[',']'> {};

//// Postfix
struct FunctionCallOp	: p::seq	< p::one<'('>, OptWs, ListOfFunctionArguments, OptWs, p::one<')'> > {};
struct EmptyFunctionCallOp		: p::string<'(',')'> {};

// Math operators
//// Prefix
struct PreincrementOp	: p::string	<'+','+'> {};
struct PredecrementOp	: p::string	<'-','-'> {};
struct NumericNegateOp	: p::one	<'-'> {};

//// Postfix
struct PostincrementOp	: p::string	<'+','+'> {};
struct PostdecrementOp	: p::string	<'-','-'> {};

//// Infix
struct AddOp			: p::string	<'+'> {};
struct SubOp			: p::string	<'-'> {};
struct MultOp			: p::string	<'*'> {};
struct DivOp			: p::string	<'/'> {};
struct ModOp			: p::string	<'%'> {};

struct AddEqOp			: p::string	<'+','='> {};
struct SubEqOp			: p::string	<'-','='> {};
struct MultEqOp			: p::string	<'*','='> {};
struct DivEqOp			: p::string	<'/','='> {};
struct ModEqOp			: p::string	<'%','='> {};

// Bitwise operators
//// Prefix
struct BitNegate		: p::one	<'~'> {};

//// Infix
struct BitAndOp			: p::string	<'&'> {};
struct BitXorOp			: p::string	<'^'> {};
struct BitOrOp			: p::string	<'|'> {};
struct BitShLeftOp		: p::string	<'<','<'> {};
struct BitShRightOp		: p::string	<'>','>'> {};

struct BitAndEqOp		: p::string	<'&','='> {};
struct BitXorEqOp		: p::string	<'^','='> {};
struct BitOrEqOp		: p::string	<'|','='> {};
struct BitShLeftEqO		: p::string	<'<','<','='> {};
struct BitShRightEqOp	: p::string	<'>','>','='> {};

// Logical operators
//// Prefix
struct LogicalNegateOp	: p::one	<'!'> {};

//// Infix
struct LogicalAndOp		: p::string<'&','&'> {};
struct LogicalOrOp		: p::string<'|','|'> {};

// Relational operators
//// Infix
struct EqualOp			: p::string	<'=','='> {};
struct InequalOp		: p::string	<'!','='> {};
struct GreaterThanOp	: p::string	<'>'> {};
struct LowerThanOp		: p::string	<'<'> {};
struct GreaterOrEqOp	: p::string	<'>','='> {};
struct LowerOrEqOp		: p::string	<'<','='> {};

// Ternary conditional:
//// Infix
struct TernaryFirstOp	: p::one	<'?'> {};
struct TernarySecondOp	: p::one	<':'> {};

// Scope operator
//// Infix
struct ScopeOp : p::two<':'>{};

// Conversion operator
//// Infix
struct ConversionOp : AsKeyword {};


///////////////////// Grammar ////////////////////////


struct PrefixOperator
	: p::sor<
			DereferenceOp,
			ReferenceOp,
			PreincrementOp,
			PredecrementOp,
			// NumericNegateOp,
			BitNegate,
			LogicalNegateOp
		>
{
};

template <typename... Optionals>
struct InfixOperatorBase
	:
		p::sor<
			AddEqOp,
			SubEqOp,
			MultEqOp,
			DivEqOp,
			ModEqOp,

			AddOp,
			SubOp,
			MultOp,
			DivOp,
			ModOp,

			BitAndEqOp,
			BitXorEqOp,
			BitOrEqOp,
			BitShLeftEqO,
			BitShRightEqOp,

			BitAndOp,
			BitXorOp,
			BitOrOp,
			BitShLeftOp,
			BitShRightOp,

			LogicalAndOp,
			LogicalOrOp,

			EqualOp,
			InequalOp,
			GreaterOrEqOp,
			LowerOrEqOp,
			GreaterThanOp,
			LowerThanOp,

			AssignOp,

			ScopeOp,

			ConversionOp,

			TernaryFirstOp,
			TernarySecondOp,

			DotOp,

			Optionals...
		>
{
};


struct InfixOperator		: InfixOperatorBase<CommaOp> {};
struct InfixOperatorNoComma	: InfixOperatorBase<> {};


struct PostfixOperator
	: p::sor<
			p::sor<SubscriptOp, EmptySubscriptOp>,
			p::sor<FunctionCallOp, EmptyFunctionCallOp>,
			PostincrementOp,
			PostdecrementOp
		>
{};


}
