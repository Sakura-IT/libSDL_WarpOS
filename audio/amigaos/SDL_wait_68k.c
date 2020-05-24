

#include <exec/avl.h>
#include <exec/ports.h>
#include <inline/exec_protos.h>

void Wait68k( __reg("a0") struct AHIRequest *req, __reg("a1") struct MsgPort *msg )
{
	struct ExecBase *SysBase;
	SysBase = *(struct ExecBase **)4L;

	WaitIO((struct IORequest *)req);

	GetMsg(msg);
	GetMsg(msg);
}
