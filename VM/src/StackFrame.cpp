#include RIGCVM_PCH

#include <RigCVM/StackFrame.hpp>
#include <RigCVM/VM.hpp>

namespace rigc::vm
{

////////////////////////////////////////
StackFramePusher::StackFramePusher(Instance& vm_, ParserNode const& stmt_)
	: vm(vm_)
{
#if DEBUG
	vm.pushStackFrameOf(&stmt_, fmt::format("{} -> {}", stmt_.type, stmt_.string()));
#else
	vm.pushStackFrameOf(&stmt_);
#endif
}

////////////////////////////////////////
StackFramePusher::~StackFramePusher()
{
	if (!vm.returnTriggered)
		vm.popStackFrame();
}


}
