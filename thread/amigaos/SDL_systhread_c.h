/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002  Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id$";
#endif

#if defined(WARPOS)
#pragma pack(2)
#endif

#include <exec/exec.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#if defined (__SASC) || defined(WARPOS) || defined(AROS)
#include <proto/dos.h>
#include <proto/exec.h>
#else
#include <inline/dos.h>
#include <inline/exec.h>
#endif

extern struct ExecBase *SysBase;
extern struct DosLibrary *DOSBase;

#pragma pack()

#include <stdio.h>
#ifndef AROS
#include <stdlib.h>
#endif
#include <string.h>
#include "../../mydebug.h"


#ifdef WARPOS
#include <powerpc/powerpc.h>
#include <powerpc/semaphoresPPC.h>

/* use powerpc.library functions instead of exec */
#define SYS_ThreadHandle struct TaskPPC *

#undef Signal
#undef Wait
#undef Task
#undef FindTask
#undef SetSignal
#undef InitSemaphore
#undef ObtainSemaphore
#undef AttemptSemaphore
#undef ReleaseSemaphore
#undef SignalSemaphore
#undef AllocSignal
#undef FreeSignal
#undef ObtainSemaphoreShared
#undef REMOVE
#undef NEWLIST
#undef ADDTAIL

#define Signal SignalPPC
#define Wait WaitPPC
#define Task TaskPPC
#define FindTask FindTaskPPC
#define SetSignal SetSignalPPC
#define InitSemaphore InitSemaphorePPC
#define ObtainSemaphore ObtainSemaphorePPC
#define AttemptSemaphore AttemptSemaphorePPC
#define ReleaseSemaphore ReleaseSemaphorePPC
#define SignalSemaphore SignalSemaphorePPC
#define AllocSignal AllocSignalPPC
#define FreeSignal FreeSignalPPC
#define ObtainSemaphoreShared ObtainSemaphoreSharedPPC
#define REMOVE RemovePPC
#define NEWLIST NewListPPC
#define ADDTAIL AddTailPPC
#else

#define SYS_ThreadHandle struct Task *
#endif /* WARPOS */

