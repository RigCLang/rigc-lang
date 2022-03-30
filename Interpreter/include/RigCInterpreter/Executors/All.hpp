#pragma once

#include RIGCINTERPRETER_PCH

namespace rigc::vm
{

struct Instance;

using OptValue = std::optional<struct Value>;

using ExecutorTrigger	= std::string_view;
using ExecutorFunction	= OptValue(Instance&, rigc::ParserNode const&);

extern std::map<ExecutorTrigger, ExecutorFunction*, std::less<> > Executors;

#define DECLARE_EXECUTOR(Name) \
	OptValue Name(Instance &vm_, rigc::ParserNode const& stmt_)

DECLARE_EXECUTOR(executeCodeBlock);
DECLARE_EXECUTOR(executeSingleStatement);
DECLARE_EXECUTOR(executeIfStatement);
DECLARE_EXECUTOR(executeWhileStatement);
DECLARE_EXECUTOR(executeReturnStatement);

DECLARE_EXECUTOR(evaluateFunctionDefinition);
DECLARE_EXECUTOR(evaluateFunctionCall);
DECLARE_EXECUTOR(evaluateExpression);

DECLARE_EXECUTOR(evaluateName);
DECLARE_EXECUTOR(evaluateIntegerLiteral);
DECLARE_EXECUTOR(evaluateBoolLiteral);
DECLARE_EXECUTOR(evaluateFloat32Literal);
DECLARE_EXECUTOR(evaluateFloat64Literal);
DECLARE_EXECUTOR(evaluateStringLiteral);
DECLARE_EXECUTOR(evaluateArrayLiteral);
DECLARE_EXECUTOR(evaluateArrayElement);

DECLARE_EXECUTOR(evaluateClassDefinition);
DECLARE_EXECUTOR(evaluateMethodDefinition);


DECLARE_EXECUTOR(evaluateVariableDefinition);

#undef DECLARE_EXECUTOR

}
