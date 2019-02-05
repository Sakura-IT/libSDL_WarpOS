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

/* An implementation of semaphores using mutexes and condition variables */

#include "SDL_error.h"
#include "SDL_thread.h"
#include "SDL_systhread_c.h"
#include "SDL_timer.h"

#ifdef _AROS
#include <stdlib.h>
#endif


struct waitnode
{
	struct MinNode node;
	struct Task   *task;
	ULONG          sigmask;
};

struct SDL_semaphore
{
	struct SignalSemaphore sem;
	struct MinList         waitlist;
	int                    sem_value;
};

#define FALLBACKSIGNAL SIGBREAKB_CTRL_E

#undef D

#define D(x)

/* Create a semaphore, initialized with value */
SDL_sem *SDL_CreateSemaphore(Uint32 initial_value)
{
	SDL_sem *sem;

	D(bug("[SDL] SDL_CreateSemaphore(%ld)\n", initial_value));

	sem = malloc(sizeof(*sem));
	if (sem)
	{
		memset(&sem->sem, 0, sizeof(sem->sem));
		InitSemaphore(&sem->sem);

		NEWLIST((struct List *)&sem->waitlist);

		sem->sem_value = initial_value;
	}
	else
	{
		SDL_OutOfMemory();
	}

	return sem;
}


void SDL_DestroySemaphore(SDL_sem *sem)
{
	D(bug("[SDL] SDL_DestroySemaphore()\n"));
	D(bug("Destroying semaphore %lx...\n",sem));

	if (sem)
	{

		ObtainSemaphore(&sem->sem);

		sem->sem_value = -1;

		while (!IsListEmpty((struct List *) &sem->waitlist))
		{
			struct waitnode *wn;
			int res;

			D(bug("[SDL] bug, semaphore %lx busy!\n", sem));

			for (wn = (struct waitnode *) sem->waitlist.mlh_Head;
			     wn->node.mln_Succ;
			     wn = (struct waitnode *) wn->node.mln_Succ)
			{
				Signal(wn->task, wn->sigmask);
			}

			ReleaseSemaphore(&sem->sem);

			res = WaitTime(SIGBREAKF_CTRL_C,10000);

			ObtainSemaphore(&sem->sem);

			if (res & SIGBREAKF_CTRL_C)
			{
				break;
			}
		}

		ReleaseSemaphore(&sem->sem);

	free(sem);
	}
}

int SDL_SemTryWait(SDL_sem *sem)
{
	int retval;

	D(bug("[SDL] SDL_SemTryWait()\n"));

	if (!sem)
	{
		SDL_SetError("Passed a NULL semaphore");
		return -1;
	}

	ObtainSemaphore(&sem->sem);

	if (sem->sem_value > 0)
	{
		--sem->sem_value;
		retval = 0;
	}
	else
	{
		retval = SDL_MUTEX_TIMEDOUT;
	}

	ReleaseSemaphore(&sem->sem);

	return retval;
}

int SDL_SemWait(SDL_sem *sem)
{
	int retval;
	struct waitnode wn;
	LONG signal;

	D(bug("[SDL] SDL_SemWait(0x%08.8lx) from thread 0x%08.8lx\n", sem, FindTask(NULL)));

	if (!sem)
	{
		SDL_SetError("Passed a NULL semaphore");
		return -1;
	}

	retval = 0;
	signal = -1;

	ObtainSemaphore(&sem->sem);

	D(bug("[SDL] SDL_SemWait(): from thread 0x%08.8lx initial sem_value: %ld\n", FindTask(NULL), sem->sem_value));

	while (sem->sem_value <= 0)
	{
		ULONG sigmask;

		if (signal == -1)
		{
			wn.task = FindTask(NULL);
			signal = AllocSignal(-1);
			if (signal == -1)
			{
				signal = FALLBACKSIGNAL;
				SetSignal(1 << FALLBACKSIGNAL, 0);
			}
			wn.sigmask = 1 << signal;

			ADDTAIL((struct List *)&sem->waitlist, (struct Node *)&wn);
		}

		D(bug("[SDL] SDL_SemWait(): waiting for semaphore... \n"));

		ReleaseSemaphore(&sem->sem);

		sigmask = Wait(wn.sigmask | SIGBREAKF_CTRL_C);

		ObtainSemaphore(&sem->sem);

		if (sigmask & SIGBREAKF_CTRL_C)
		{
			D(bug("[SDL] SDL_SemWait(): was aborted...\n"));
			retval = -1;
			break;
		}
	}

	if (signal != -1)
	{
		REMOVE((struct Node *)&wn);

		if (signal != FALLBACKSIGNAL)
		{
			FreeSignal(signal);
		}
	}

	if (retval == 0)
	{
		--sem->sem_value;

		D(bug("[SDL] SDL_SemWait(): final sem_value: %ld\n", sem->sem_value));
	}

	ReleaseSemaphore(&sem->sem);
	D(bug("done...\n"));

	return retval;
}

int SDL_SemWaitTimeout(SDL_sem *sem, Uint32 timeout)
{
	int retval;

	D(bug("[SDL] SDL_SemWaitTimeout()\n"));

	if (!sem)
	{
		SDL_SetError("Passed a NULL semaphore");
		return -1;
	}

	/* Try the easy cases first */
	if (timeout == 0)
	{
		return SDL_SemTryWait(sem);
	}
	if (timeout == SDL_MUTEX_MAXWAIT)
	{
		return SDL_SemWait(sem);
	}

	struct waitnode wn;
	LONG signal;

	retval = 0;
	signal = -1;

	ObtainSemaphore(&sem->sem);

	while (sem->sem_value <= 0)
	{
		if (signal == -1)
		{
		wn.task = FindTask(NULL);
		signal = AllocSignal(-1);
			if (signal == -1)
			{
				signal = FALLBACKSIGNAL;
				SetSignal(1 << FALLBACKSIGNAL, 0);
			}
			wn.sigmask = 1 << signal;

			ADDTAIL((struct List *)&sem->waitlist, (struct Node *)&wn);
		}

		ReleaseSemaphore(&sem->sem);

		retval = WaitTime(wn.sigmask || SIGBREAKF_CTRL_C, (timeout*1000));

		ObtainSemaphore(&sem->sem);

		/* CTRL-C? */
		if (retval & SIGBREAKF_CTRL_C)
		{
			break;
		}

		/* Timed out? (== no 'semaphore released'-signal) */
		if (retval == 0)
		{
			retval = SDL_MUTEX_TIMEDOUT;

			break;
		}

 		/*
		 * Not timed out, someone released the semaphore.
		 * See if we can get it. If we can't, continue waiting.
		 */
	}

	if (signal != -1)
	{
		REMOVE((struct Node *)&wn);

		if (signal != FALLBACKSIGNAL)
		{
			FreeSignal(signal);
		}
		
	}

	/* If can obtain (no timeout, no break), obtain it */
	if (retval == wn.sigmask)
	{
		--sem->sem_value;

		D(bug("[SDL] SDL_SemWaitTimeout(): final sem_value: %ld\n", sem->sem_value));
	}

	ReleaseSemaphore(&sem->sem);

	return retval;
}

Uint32 SDL_SemValue(SDL_sem *sem)
{
	int retval;

	D(bug("[SDL] SDL_SemValue()\n"));

	if (!sem)
	{
		SDL_SetError("Passed a NULL semaphore");
		return 0;
	}

	ObtainSemaphoreShared(&sem->sem);
	
	retval = sem->sem_value;

	ReleaseSemaphore(&sem->sem);

	if (retval < 0)
	{
		retval = 0;
	}

	return (Uint32) retval;
}

int SDL_SemPost(SDL_sem *sem)
{
	struct waitnode *wn;

	if (!sem)
	{
		SDL_SetError("Passed a NULL semaphore");
		return -1;
	}

	ObtainSemaphore(&sem->sem);

	D(bug("[SDL] SDL_SemPost(0x%08.8lx): initial sem_value: %ld\n", sem, sem->sem_value));

	#if 1

	/* Wake whatever task happens to be first in the waitlist */
	wn = (struct waitnode *) sem->waitlist.mlh_Head;
	if (wn->node.mln_Succ)
	{
		Signal(wn->task, wn->sigmask);
	}

	#else

	for (wn = (struct waitnode *) sem->waitlist.mlh_Head;
	     wn->node.mln_Succ;
	     wn = (struct waitnode *) wn->node.mln_Succ)
	{
		Signal(wn->task, wn->sigmask);
	}

	#endif

	++sem->sem_value;

	D(bug("[SDL] SDL_SemPost(): final sem_value: %ld\n", sem->sem_value));

	ReleaseSemaphore(&sem->sem);

	return 0;
}

