#include RIGCVM_PCH

#include <RigCVM/Executors/All.hpp>
#include <RigCVM/VM.hpp>

#include <RigCVM/TypeSystem/ArrayType.hpp>

namespace rigc::vm
{

////////////////////////////////////////
static auto replaceAll(std::string& s, std::string_view from, std::string_view to) -> void
{
	size_t startPos = 0;
	while((startPos = s.find(from, startPos)) != std::string::npos) {
		s.replace(startPos, from.length(), to);
		startPos += to.length();
	}
}

////////////////////////////////////////
auto evaluateIntegerLiteral(Instance &vm_, rigc::ParserNode const& expr_) -> OptValue
{
	return vm_.allocateOnStack<int>( "Int32", std::stoi(expr_.string()) );
}

////////////////////////////////////////
auto evaluateFloat32Literal(Instance &vm_, rigc::ParserNode const& expr_) -> OptValue
{
	auto n = expr_.string();
	return vm_.allocateOnStack<float>( "Float32", std::stof( n.substr(0, n.size() - 1) ) );
}

////////////////////////////////////////
auto evaluateFloat64Literal(Instance &vm_, rigc::ParserNode const& expr_) -> OptValue
{
	return vm_.allocateOnStack<double>( "Float64", std::stod(expr_.string()) );
}

////////////////////////////////////////
auto evaluateBoolLiteral(Instance &vm_, rigc::ParserNode const& expr_) -> OptValue
{
	return vm_.allocateOnStack<bool>( "Bool", expr_.string_view()[0] == 't' ? true : false);
}

////////////////////////////////////////
auto evaluateStringLiteral(Instance &vm_, rigc::ParserNode const& expr_) -> OptValue
{
	auto sv = expr_.string_view();

	std::string s(sv, 1, sv.length() - 2);
	s.reserve(s.size() * 2);
	replaceAll(s, "\\n",	"\n");
	replaceAll(s, "\\t",	"\t");
	replaceAll(s, "\\r",	"\r");
	replaceAll(s, "\\a",	"\a");
	replaceAll(s, "\\v",	"\v");
	replaceAll(s, "\\\\",	"\\");
	replaceAll(s, "\\\"",	"\"");

	auto type = constructTemplateType<ArrayType>(vm_.universalScope(), vm_.findType("Char")->shared_from_this(), s.size());

	return vm_.allocateOnStack( std::move(type), s.data(), s.size() );
}

////////////////////////////////////////
auto evaluateCharLiteral(Instance &vm_, rigc::ParserNode const& expr_) -> OptValue
{
	auto sv = expr_.string_view();

	std::string s(sv, 1, sv.length() - 2);
	replaceAll(s, "\\n",	"\n");
	replaceAll(s, "\\t",	"\t");
	replaceAll(s, "\\r",	"\r");
	replaceAll(s, "\\a",	"\a");
	replaceAll(s, "\\v",	"\v");
	replaceAll(s, "\\\\",	"\\");
	replaceAll(s, "\\\"",	"\"");
	char c = s[0];

	return vm_.allocateOnStack( vm_.findType("Char")->shared_from_this(), c );
}

// ////////////////////////////////////////
// auto evaluateArrayLiteral(Instance &vm_, rigc::ParserNode const& expr_) -> OptValue
// {
// 	std::vector<Value> arr;
// 	arr.reserve(expr_.children.size());

// 	for (auto const& c : expr_.children)
// 	{
// 		arr.push_back( vm_.evaluate(*c).value() );
// 	}

// 	Value v( std::move(arr) );

// 	// vm_.stack.push( v );
// 	return v;
// }

// ////////////////////////////////////////
// auto evaluateArrayElement(Instance &vm_, rigc::ParserNode const& expr_) -> OptValue
// {
// 	Value v( vm_.evaluate(*expr_.children[0]).value() );

// 	// vm_.stack.push( v );
// 	return v;
// }


}
