#include RIGCVM_PCH

#include <RigCVM/StackFrame.hpp>
#include <RigCVM/VM.hpp>

namespace rigc::vm
{

////////////////////////////////////////
StackFramePusher::StackFramePusher(Instance& vm_, ParserNode const& stmt_)
	: vm(vm_)
{
	vm.pushStackFrameOf(&stmt_);
}

////////////////////////////////////////
StackFramePusher::~StackFramePusher()
{
	if (!vm.returnTriggered)
		vm.popStackFrame();
}


}
