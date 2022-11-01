#pragma once

#include <RigCVM/RigCVMPCH.hpp>

namespace rigc::vm
{

struct Instance;

using OptValue = std::optional<struct Value>;

using ExecutorTrigger	= StringView;
using ExecutorFunction	= OptValue(Instance&, rigc::ParserNode const&);

extern Map<ExecutorTrigger, ExecutorFunction*, std::less<> > Executors;

#define DECLARE_EXECUTOR(Name) \
	auto Name(Instance &vm_, rigc::ParserNode const& stmt_) -> OptValue;

DECLARE_EXECUTOR(executeImportStatement);
DECLARE_EXECUTOR(executeCodeBlock);
DECLARE_EXECUTOR(executeSingleStatement);
DECLARE_EXECUTOR(executeIfStatement);
DECLARE_EXECUTOR(executeWhileStatement);
DECLARE_EXECUTOR(executeForStatement);
DECLARE_EXECUTOR(executeReturnStatement);

DECLARE_EXECUTOR(evaluateBreakStatement);
DECLARE_EXECUTOR(evaluateContinueStatement);

DECLARE_EXECUTOR(evaluateFunctionDefinition);
DECLARE_EXECUTOR(evaluateExpression);

DECLARE_EXECUTOR(evaluateSymbol);
DECLARE_EXECUTOR(evaluateName);
DECLARE_EXECUTOR(evaluateIntegerLiteral);
DECLARE_EXECUTOR(evaluateBoolLiteral);
DECLARE_EXECUTOR(evaluateFloat32Literal);
DECLARE_EXECUTOR(evaluateFloat64Literal);
DECLARE_EXECUTOR(evaluateStringLiteral);
DECLARE_EXECUTOR(evaluateCharLiteral);
DECLARE_EXECUTOR(evaluateArrayLiteral);
DECLARE_EXECUTOR(evaluateArrayElement);

DECLARE_EXECUTOR(executeUnionDefinition);
DECLARE_EXECUTOR(executeEnumDefinition);

DECLARE_EXECUTOR(evaluateClassDefinition);
DECLARE_EXECUTOR(evaluateMethodDefinition);
DECLARE_EXECUTOR(evaluateMemberOperatorDefinition);
DECLARE_EXECUTOR(evaluateDataMemberDefinition);


DECLARE_EXECUTOR(evaluateVariableDefinition);

#undef DECLARE_EXECUTOR

}
