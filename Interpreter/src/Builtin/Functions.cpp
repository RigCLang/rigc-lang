#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Builtin/Functions.hpp>

#include <RigCInterpreter/TypeSystem/RefType.hpp>
#include <RigCInterpreter/VM.hpp>

namespace rigc::vm::builtin
{

////////////////////////////////////////
OptValue print(Instance &vm_, Function::Args& args_, size_t argCount_)
{
	if (argCount_ == 0)
		return {};

	auto format = args_[0];
	if (!format.getType()->isArray() && format.typeName() != "Char")
		return {};

	auto chars = &format.view<char const>();
	auto fmtView = std::string_view(chars, format.getType()->size());

	auto store = fmt::dynamic_format_arg_store<fmt::format_context>();
	for (size_t c = 1; c < argCount_; ++c)
	{
		Value val = args_[c].safeRemoveRef();

		DeclType const& type = val.getType();

		auto typeName = val.typeName();
		if (typeName == "Int32")
			store.push_back(val.view<int>());
		else if (typeName == "Float32")
			store.push_back(val.view<float>());
		else if (typeName == "Float64")
			store.push_back(val.view<double>());
		else if (typeName == "Bool")
			store.push_back((val.view<bool>() ? "true" : "false"));
		else if (val.getType()->isArray() && typeName == "Char")
		{
			auto chars = &val.view<char const>();

			store.push_back(std::string(chars, type->size()));
		}
		else if (typeName == "Char")
			store.push_back(val.view<char>());
		else
			store.push_back(dump(vm_, val));
	}

	fmt::vprint(fmtView, store);

	return {};
}

}

