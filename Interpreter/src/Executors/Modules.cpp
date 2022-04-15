#include RIGCINTERPRETER_PCH

#include <RigCInterpreter/Executors/All.hpp>
#include <RigCInterpreter/VM.hpp>

namespace rigc::vm
{

////////////////////////////////////////
OptValue executeImportStatement(Instance &vm_, rigc::ParserNode const& stmt_)
{
	auto moduleName = findElem<rigc::PackageImportFullName>(stmt_)->string_view();
	moduleName = moduleName.substr(1, moduleName.size() - 2);

	// fmt::print("Importing module {}...\n\n", moduleName);

	auto path = vm_.findModulePath(moduleName);
	if (vm_.loadedModules.contains(path))
	{
		// fmt::print("Module {} already loaded.\n\n", moduleName);
		return {};
	}

	if (auto mod = vm_.parseModule(moduleName))
	{
		vm_.evaluateModule(*mod);
		return {};
	}
	throw std::runtime_error(fmt::format("Module {} not found", moduleName));
}

}
