#pragma once

#include RIGCVM_PCH

#include <RigCVM/Scope.hpp>

namespace rigc::vm
{
class Module : public Scope
{
public:
	enum class State {
		Unresolved,
		Pending,
		Parsed,
		Loaded
	};

	Module(struct Instance& vm_)
		: Scope(vm_)
	{
	}

	UniquePtr<rigc::ParserNode const>	root;
	UniquePtr<pegtl::file_input<>>		fileInput;
	FsPath								absolutePath;
	DynArray<Module*>					importedModules;

	State state = State::Unresolved;
};
}
