#include "VM/include/RigCVM/RigCVMPCH.hpp"

#include <RigCVM/Builtin/Functions.hpp>

#include <RigCVM/TypeSystem/RefType.hpp>
#include <RigCVM/TypeSystem/TemplateType.hpp>
#include <RigCVM/TypeSystem/ArrayType.hpp>
#include <RigCVM/VM.hpp>

namespace rigc::vm::builtin
{

auto allocateMemory(Instance &vm_, Function::ArgSpan args_) -> OptValue
{
	if (args_.size() != 1)
		return OptValue();

	auto size = args_[0].safeRemoveRef().view<int>();

	auto mem = std::malloc(size);
	auto type = vm_.builtinTypes.Char.shared();
	return vm_.allocatePointer( Value { type, mem } );
}

auto freeMemory(Instance &vm_, Function::ArgSpan args_) -> OptValue
{
	if (args_.size() != 1)
		return OptValue();

	std::free(args_[0].safeRemoveRef().removePtr().data);
	return {};
}

auto printCharacters(Instance &vm_, Function::ArgSpan args_) -> OptValue
{
	// Detect VM malfunction. Overload resolution should have prevented this.
	assert((args_.size() == 2) && "builtin::printCharacters() called with wrong number of arguments.");

	auto chars = &args_[0].removePtr().view<char>();
	auto size = args_[1].view<int>();

	vm_.print("{}", StringView(chars, size_t(size)));

	return std::nullopt;
}

////////////////////////////////////////
auto print(Instance &vm_, Function::ArgSpan args_) -> OptValue
{
	if (args_.size() == 0)
		return {};

	auto format = args_[0];
	if (!format.getType()->isArray() && format.typeName() != "Char")
		return {};

	auto chars = &format.view<char const>();
	auto fmtView = StringView(chars, format.getType()->size());

	auto store = fmt::dynamic_format_arg_store<fmt::format_context>();
	store.reserve(args_.size() - 1, 0);
	for (size_t c = 1; c < args_.size(); ++c)
	{
		Value val = args_[c].safeRemoveRef();

		DeclType const& type = val.getType();

		auto typeName = val.type->name();
		auto decayedTypeName = val.typeName();
		if (typeName == "Int32")
			store.push_back(val.view<int>());
		else if (typeName == "Char")
			store.push_back(val.view<char>());
		else if (typeName == "Float32")
			store.push_back(val.view<float>());
		else if (typeName == "Float64")
			store.push_back(val.view<double>());
		else if (typeName == "Bool")
			store.push_back((val.view<bool>() ? "true" : "false"));
		else if (val.type->is<AddrType>())
		{
			auto& addrType = val.type->getTemplateArguments()[0].as<DeclType>();
			if (addrType.get() == vm_.builtinTypes.Void.raw)
			{
				if constexpr (sizeof(void*) == 8) {
					store.push_back(fmt::format("0x{:0>16x}", uintptr_t(val.view<void*>())));
				} else {
					store.push_back(fmt::format("0x{:0>8x}", uintptr_t(val.view<void*>())));
				}
			}
			else if (addrType.get() == vm_.builtinTypes.Char.raw)
			{
				store.push_back(val.view<char*>());
			}
			else
			{
				throw RigCError("Invalid type {} for \"print\" (builtin) function.", val.type->name())
					.withHelp(
						"Convert the address to Addr<Void> to print an address or Addr<Char> to print a null-terminated string.\n"
						"[Example]:\n"
						"    print(\"{{}}\", addr as Addr::<Void>);\n"
					)
					.withLine(vm_.lastEvaluatedLine);
			}
		}
		else if (val.getType()->isArray() && decayedTypeName == "Char")
		{
			auto chars = &val.view<char const>();

			store.push_back(String(chars, type->size()));
		}
		// else if (typeName == "Char")
		// 	store.push_back(val.view<char>());
		else
			store.push_back(dump(vm_, val));
	}

	vm_.std_out() << fmt::vformat(fmtView, store);

	return {};
}

////////////////////////////////////////
auto dumpTypeOf(Instance &vm_, Function::ArgSpan args_) -> OptValue
{
	auto name = args_[0].safeRemoveRef().type->name();
	fmt::print("{}", name);

	return std::nullopt;
}

////////////////////////////////////////
auto printMessage(Value const& msg) -> void
{
	// FIXME: for now just accepting the type Array<Char, Size> cuz we cant do anything else
	if (!msg.getType()->isArray() && msg.typeName() != "Char")
		return;

	auto chars = &msg.view<const char>();

	fmt::print("{}", StringView(chars, msg.getType()->size()));
}

////////////////////////////////////////
template <typename CppType>
auto executeRead(Instance &vm_, StringView rigcTypeName) -> OptValue
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
