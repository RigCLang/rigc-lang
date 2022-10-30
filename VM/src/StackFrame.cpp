#include "VM/include/RigCVM/RigCVMPCH.hpp"

#include <RigCVM/StackFrame.hpp>
#include <RigCVM/VM.hpp>
#include <RigCVM/DevServer/Utils.hpp>

namespace rigc::vm
{

////////////////////////////////////////
StackFramePusher::StackFramePusher(Instance& vm_, ParserNode const& stmt_)
	: vm(vm_)
{
#if DEBUG
	vm.pushStackFrameOf(&stmt_, formatStackFrameLabel(stmt_));
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
