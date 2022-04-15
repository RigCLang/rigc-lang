#pragma once

#include RIGCVM_PCH

namespace rigc::vm
{

struct Instance;

using OptValue = std::optional<struct Value>;

using ExecutorTrigger	= std::string_view;
using ExecutorFunction	= OptValue(Instance&, rigc::ParserNode const&);

extern std::map<ExecutorTrigger, ExecutorFunction*, std::less<> > Executors;

struct StackFramePusher
{
	StackFramePusher(Instance& vm_, ParserNode const& stmt_);
	~StackFramePusher();

	Instance& vm;
};

#define DECLARE_EXECUTOR(Name) \
	OptValue Name(Instance &vm_, rigc::ParserNode const& stmt_)

DECLARE_EXECUTOR(executeImportStatement);
DECLARE_EXECUTOR(executeCodeBlock);
DECLARE_EXECUTOR(executeSingleStatement);
DECLARE_EXECUTOR(executeIfStatement);
DECLARE_EXECUTOR(executeWhileStatement);
DECLARE_EXECUTOR(executeReturnStatement);

DECLARE_EXECUTOR(evaluateFunctionDefinition);
DECLARE_EXECUTOR(evaluateExpression);

DECLARE_EXECUTOR(evaluateName);
DECLARE_EXECUTOR(evaluateIntegerLiteral);
DECLARE_EXECUTOR(evaluateBoolLiteral);
DECLARE_EXECUTOR(evaluateFloat32Literal);
DECLARE_EXECUTOR(evaluateFloat64Literal);
DECLARE_EXECUTOR(evaluateStringLiteral);
DECLARE_EXECUTOR(evaluateCharLiteral);
DECLARE_EXECUTOR(evaluateArrayLiteral);
DECLARE_EXECUTOR(evaluateArrayElement);

DECLARE_EXECUTOR(evaluateClassDefinition);
DECLARE_EXECUTOR(evaluateMethodDefinition);
DECLARE_EXECUTOR(evaluateDataMemberDefinition);


DECLARE_EXECUTOR(evaluateVariableDefinition);

#undef DECLARE_EXECUTOR

}
