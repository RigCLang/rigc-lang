#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Executors/All.hpp>
#include <RigCInterpreter/VM.hpp>

#include <RigCInterpreter/TypeSystem/ArrayType.hpp>

namespace rigc::vm
{

////////////////////////////////////////
static void replaceAll(std::string& s, std::string_view from, std::string_view to)
{
	size_t startPos = 0;
	while((startPos = s.find(from, startPos)) != std::string::npos) {
		s.replace(startPos, from.length(), to);
		startPos += to.length();
	}
}

////////////////////////////////////////
OptValue evaluateIntegerLiteral(Instance &vm_, rigc::ParserNode const& expr_)
{
	return vm_.allocateOnStack<int>( "Int32", std::stoi(expr_.string()) );
}

////////////////////////////////////////
OptValue evaluateFloat32Literal(Instance &vm_, rigc::ParserNode const& expr_)
{
	auto n = expr_.string();
	return vm_.allocateOnStack<float>( "Float32", std::stof( n.substr(0, n.size() - 1) ) );
}

////////////////////////////////////////
OptValue evaluateFloat64Literal(Instance &vm_, rigc::ParserNode const& expr_)
{
	return vm_.allocateOnStack<double>( "Float64", std::stod(expr_.string()) );
}

////////////////////////////////////////
OptValue evaluateBoolLiteral(Instance &vm_, rigc::ParserNode const& expr_)
{
	return vm_.allocateOnStack<bool>( "Bool", expr_.string_view()[0] == 't' ? true : false);
}

////////////////////////////////////////
OptValue evaluateStringLiteral(Instance &vm_, rigc::ParserNode const& expr_)
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

	auto type = wrap<ArrayType>(vm_.universalScope(), vm_.findType("Char")->shared_from_this(), s.size());

	vm_.universalScope().addType(type);

	return vm_.allocateOnStack( std::move(type), s.data(), s.size() );
}

////////////////////////////////////////
OptValue evaluateCharLiteral(Instance &vm_, rigc::ParserNode const& expr_)
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
// OptValue evaluateArrayLiteral(Instance &vm_, rigc::ParserNode const& expr_)
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
// OptValue evaluateArrayElement(Instance &vm_, rigc::ParserNode const& expr_)
// {
// 	Value v( vm_.evaluate(*expr_.children[0]).value() );

// 	// vm_.stack.push( v );
// 	return v;
// }


}
