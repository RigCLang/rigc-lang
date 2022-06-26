#include RIGCVM_PCH

#include <RigCVM/Executors/All.hpp>
#include <RigCVM/VM.hpp>

#include <RigCVM/ErrorHandling/Exceptions.hpp>

namespace rigc::vm
{

////////////////////////////////////////
auto executeImportStatement(Instance &vm_, rigc::ParserNode const& stmt_) -> OptValue
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
	throw RigcException("Module {} not found", moduleName)
					.withHelp("Try checking the name of the module, the path of the module or try declaring it.")
					.withLine(vm_.lastEvaluatedLine);
}

}
