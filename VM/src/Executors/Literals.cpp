#include "VM/include/RigCVM/RigCVMPCH.hpp"

#include <RigCVM/Executors/All.hpp>
#include <RigCVM/VM.hpp>

#include <RigCVM/TypeSystem/ArrayType.hpp>
#include <RigCVM/Helper/String.hpp>

namespace rigc::vm
{

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

	String s(sv, 1, sv.length() - 2);
	s.reserve(s.size() * 2);
	replaceAll(s, "\\n",	"\n");
	replaceAll(s, "\\t",	"\t");
	replaceAll(s, "\\r",	"\r");
	replaceAll(s, "\\a",	"\a");
	replaceAll(s, "\\v",	"\v");
	replaceAll(s, "\\\\",	"\\");
	replaceAll(s, "\\\"",	"\"");


	auto type = vm_.arrayOf(*vm_.builtinTypes.Char.raw, s.size());

	return vm_.allocateOnStack( std::move(type), s.data(), s.size() );
}

////////////////////////////////////////
auto evaluateCharLiteral(Instance &vm_, rigc::ParserNode const& expr_) -> OptValue
{
	auto sv = expr_.string_view();

	auto s = String(sv, 1, sv.length() - 2);
	replaceAll(s, "\\n",	"\n");
	replaceAll(s, "\\t",	"\t");
	replaceAll(s, "\\r",	"\r");
	replaceAll(s, "\\a",	"\a");
	replaceAll(s, "\\v",	"\v");
	replaceAll(s, "\\\\",	"\\");
	replaceAll(s, "\\\"",	"\"");
	char c = s[0];

	return vm_.allocateOnStack( vm_.builtinTypes.Char.shared(), c );
}

bool copyConstructOn(Instance&, Value, Value const&);

////////////////////////////////////////
auto evaluateArrayLiteral(Instance &vm_, rigc::ParserNode const& expr_) -> OptValue
{

	std::vector<Value> arr;
	arr.reserve(expr_.children.size());

	if (expr_.children.empty())
	{
		// TODO: add support for an empty literal
		throw RigCError("Cannot deduce array type from an empty literal");
	}

	auto outArray = Value();

	for (size_t i = 0; i < expr_.children.size(); ++i)
	{
		auto v = vm_.evaluate(*expr_.children[i]).value();

		if (i == 0)
		{
			outArray = vm_.allocateOnStack( vm_.arrayOf(*v.type, expr_.children.size()), nullptr, expr_.children.size() );
		}

		auto buf = &outArray.view<char>();
		auto inplaceValue = Value();
		inplaceValue.type = v.type;
		inplaceValue.data = buf + (i * v.type->size());

		if (!copyConstructOn(vm_, inplaceValue, v))
		{
			std::memcpy(inplaceValue.data, v.data, v.type->size());
		}
	}

	return vm_.allocateReference(outArray);
}

}
