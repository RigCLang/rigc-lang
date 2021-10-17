#pragma once

#include RIGCINTERPRETER_PCH

namespace rigc::vm
{

struct Instance;

using OptValue = std::optional<struct Value>;

using ExecutorTrigger	= std::string_view;
using ExecutorFunction	= OptValue(Instance&, rigc::ParserNode const&);

extern std::map<ExecutorTrigger, ExecutorFunction*> Executors;

OptValue executeCodeBlock(Instance &inst_, rigc::ParserNode const& stmt_);
OptValue executeIfStatement(Instance &inst_, rigc::ParserNode const& stmt_);
OptValue executeWhileStatement(Instance &inst_, rigc::ParserNode const& stmt_);
OptValue executeReturnStatement(Instance &inst_, rigc::ParserNode const& stmt_);

OptValue evaluateFunctionCall(Instance &inst_, rigc::ParserNode const& stmt_);
OptValue evaluateExpression(Instance &inst_, rigc::ParserNode const& stmt_);

OptValue evaluateName(Instance &inst_, rigc::ParserNode const& stmt_);
OptValue evaluateIntegerLiteral(Instance &inst_, rigc::ParserNode const& stmt_);
OptValue evaluateStringLiteral(Instance &inst_, rigc::ParserNode const& stmt_);
OptValue evaluateArrayLiteral(Instance &inst_, rigc::ParserNode const& stmt_);
OptValue evaluateArrayElement(Instance &inst_, rigc::ParserNode const& stmt_);

OptValue evaluateVariableDefinition(Instance &inst_, rigc::ParserNode const& stmt_);

}