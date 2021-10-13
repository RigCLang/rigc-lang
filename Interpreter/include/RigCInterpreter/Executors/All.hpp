#pragma once

#include RIGCINTERPRETER_PCH

namespace rigc::vm
{

struct Instance;

using OptValue = std::optional<struct Value>;

using ExecutorTrigger	= std::string_view;
using ExecutorFunction	= OptValue(Instance&, rigc::ParserNode&);

extern std::map<ExecutorTrigger, ExecutorFunction*> Executors;

OptValue executeCodeBlock(Instance &inst_, rigc::ParserNode& stmt_);
OptValue executeIfStatement(Instance &inst_, rigc::ParserNode& stmt_);

OptValue evaluateFunctionCall(Instance &inst_, rigc::ParserNode& stmt_);
OptValue evaluateExpression(Instance &inst_, rigc::ParserNode& stmt_);

OptValue evaluateName(Instance &inst_, rigc::ParserNode& stmt_);
OptValue evaluateIntegralLiteral(Instance &inst_, rigc::ParserNode& stmt_);

}