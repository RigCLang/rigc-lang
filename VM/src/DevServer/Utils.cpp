#include RIGCVM_PCH

#include <RigCVM/VM.hpp>
#include <RigCVM/DevServer/Utils.hpp>

#include <cctype>
#include <numeric>
#include <functional>

namespace {

auto formatFunctionSignature(rigc::ParserNode const& node) {
	using namespace rigc;
	using namespace rigc::vm;

	auto const* const name = findElem<rigc::Name>(node);
	auto const* const params = findElem<rigc::FunctionParams>(node);
	auto const* const returnType = findElem<rigc::ExplicitReturnType>(node);

	// name(arg1: type, arg2: type): return_type
	return name->string()
		+ (params ? params->string() : "()")
		+ (returnType ? ": " : "")
		+ (returnType ? returnType->string() : "");
}

auto stripBlockAndWhitespace(std::string& str) {
	auto const firstAlphaCharIt = rg::find_if(str, ::isalpha);

	auto const begin = rg::begin(str);
	auto const end = begin + (firstAlphaCharIt - begin);

	str.erase(
        begin,
		end
    );
}

auto formatCodeBlock(rigc::ParserNode const& node) {
	using namespace std::string_view_literals;

	auto static constexpr maxLabelNameLen = 20;
	auto static constexpr overflowIndicator = "..."sv;
	auto static constexpr overflowIndicatorLen = overflowIndicator.length();
	auto static constexpr maxAllowedLen = maxLabelNameLen - overflowIndicatorLen;

	auto nodeString = node.string();

	if (node.is_type<rigc::CodeBlock>()) {
		stripBlockAndWhitespace(nodeString);
	}

	if(nodeString.length() > maxLabelNameLen) {
		return
			nodeString.substr(0, maxAllowedLen)
				+ std::string(overflowIndicator);
	} else {
		return nodeString;
	}
}

}

namespace rigc::vm {

auto formatStackFrameLabel(ParserNode const& node) -> std::string {
	auto const isFunctionDefinition = node.is_type<rigc::FunctionDefinition>();

	if(isFunctionDefinition) {
		return formatFunctionSignature(node);
	} else {
		return formatCodeBlock(node);
	}
}

}
