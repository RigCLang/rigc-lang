#pragma once

#include RIGCVM_PCH

#include <RigCVM/Scope.hpp>

namespace rigc::vm
{

class Module
	: public Scope
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

	std::unique_ptr<rigc::ParserNode const>	root;
	std::unique_ptr<pegtl::file_input<>>	fileInput;
	fs::path				absolutePath;
	std::vector<Module*>	importedModules;

	State state = State::Unresolved;
};


}
