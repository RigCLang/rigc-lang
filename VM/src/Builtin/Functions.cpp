#include RIGCVM_PCH

#include <RigCVM/Builtin/Functions.hpp>

#include <RigCVM/TypeSystem/RefType.hpp>
#include <RigCVM/TypeSystem/WrapperType.hpp>
#include <RigCVM/TypeSystem/ArrayType.hpp>
#include <RigCVM/VM.hpp>

namespace rigc::vm::builtin
{

////////////////////////////////////////
auto print(Instance &vm_, Function::ArgSpan args_) -> OptValue
{
	if (args_.size() == 0)
		return {};

	auto format = args_[0];
	if (!format.getType()->isArray() && format.typeName() != "Char")
		return {};

	auto chars = &format.view<char const>();
	auto fmtView = std::string_view(chars, format.getType()->size());

	auto store = fmt::dynamic_format_arg_store<fmt::format_context>();
	for (size_t c = 1; c < args_.size(); ++c)
	{
		Value val = args_[c].safeRemoveRef();

		DeclType const& type = val.getType();

		auto typeName = val.type->name();
		auto decayedTypeName = val.typeName();
		if (typeName == "Int32")
			store.push_back(val.view<int>());
		if (typeName == "Char")
			store.push_back(val.view<char>());
		else if (typeName == "Float32")
			store.push_back(val.view<float>());
		else if (typeName == "Float64")
			store.push_back(val.view<double>());
		else if (typeName == "Bool")
			store.push_back((val.view<bool>() ? "true" : "false"));
		else if (val.getType()->isArray() && decayedTypeName == "Char")
		{
			auto chars = &val.view<char const>();

			store.push_back(std::string(chars, type->size()));
		}
		// else if (typeName == "Char")
		// 	store.push_back(val.view<char>());
		else
			store.push_back(dump(vm_, val));
	}

	fmt::vprint(fmtView, store);

	return {};
}

////////////////////////////////////////
auto typeOf(Instance &vm_, Function::ArgSpan args_) -> OptValue
{
	auto name = args_[0].safeRemoveRef().type->name();
	auto t = wrap<ArrayType>(vm_.universalScope(), vm_.findType("Char")->shared_from_this(), name.size());

	return vm_.allocateOnStack( t, name.data(), name.size() );
}

////////////////////////////////////////
auto printMessage(Value const& msg) -> void
{
	// FIXME: for now just accepting the type StaticArray<Char, Size> cuz we cant do anything else
	if (!msg.getType()->isArray() && msg.typeName() != "Char")
		return;

	auto chars = &msg.view<const char>();

	fmt::print("{}", std::string_view(chars, msg.getType()->size()));
}

////////////////////////////////////////
template <typename CppType>
auto executeRead(Instance &vm_, std::string_view rigcTypeName) -> OptValue
{
	auto data = CppType();
	std::cin >> data; // scanf?

	return vm_.allocateOnStack(rigcTypeName, data);
}

////////////////////////////////////////
auto readInt(Instance &vm_, Function::ArgSpan args_) -> OptValue
{
	if(args_.size() != 0)
		printMessage(args_[0]);

	return executeRead<int>(vm_, "Int32");
}

////////////////////////////////////////
auto readFloat(Instance &vm_, Function::ArgSpan args_) -> OptValue
{
	if(args_.size() != 0)
		printMessage(args_[0]);

	return executeRead<double>(vm_, "Float64");
}

}
