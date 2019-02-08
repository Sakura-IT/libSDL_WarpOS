/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2012 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/

#include "SDL_config.h"
#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id$";
#endif

#include "SDL_syswm.h"
#include "../../events/SDL_events_c.h"
#include "SDL_cgxmodes_c.h"
#include "SDL_cgxwm_c.h"

#pragma pack(push,2)
#include <devices/input.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/wb.h>
#include <proto/icon.h>
#pragma pack(pop)


void CGX_SetIcon(_THIS, SDL_Surface *icon, Uint8 *mask)
{
	return;
}

#if 0 //for appicon. Need Show/HideWindow()
/**********************************************************************
	GetTaskName
**********************************************************************/

static STRPTR GetTaskName(STRPTR buf, LONG buflen)
{
	struct Process *task;
	STRPTR name;

	task = (struct Process *)FindTask(NULL);
	name = NULL;

	if (task->pr_Task.tc_Node.ln_Type == NT_PROCESS || task->pr_Task.tc_Node.ln_Type == NT_TASK)
	{
		if (task->pr_Task.tc_Node.ln_Type == NT_PROCESS && task->pr_CLI)
		{
			struct CommandLineInterface *cli = (struct CommandLineInterface *)BADDR(task->pr_CLI);

			if (cli->cli_Module && cli->cli_CommandName)
			{
				STRPTR src;

				src = (STRPTR)BADDR(cli->cli_CommandName);

				if (buflen >= (*src + 1) && *src)
				{
					ULONG i;

					name = buf;

					for (i = *src++; i > 0; i--)
						*buf++ = *src++;

					*buf = '\0';
				}
			}
		}
		else
		{
			if (strlen(task->pr_Task.tc_Node.ln_Name) <= (buflen - 8))
			{
				name = buf;

				strcpy(buf, "PROGDIR:");
				strcpy(buf+8, task->pr_Task.tc_Node.ln_Name);
			}
		}
	}

	return name;
}

#endif

void CGX_SetCaption(_THIS, const char *title, const char *icon)
{
	if(SDL_Window)
		SetWindowTitles(SDL_Window,(char *)title,(char *)icon);
}

/* Iconify the window */

int CGX_IconifyWindow(_THIS)
{
#if 0
	struct SDL_PrivateVideoData *hidden = this->hidden;

	D(bug("[SDL] CGX_IconifyWindow()\n"));

	if (!hidden->AppIcon && SDL_Window)
	{
		if (currently_fullscreen)
		{
			ScreenToBack(SDL_Display);
		}
		else
		{
			struct DiskObject *icon = hidden->DiskObj;

			if (!icon)
			{
				STRPTR appname;
				UBYTE  namebuf[128];

				appname = GetTaskName((STRPTR)&namebuf, sizeof(namebuf));

				switch ((ULONG)appname)
				{
					default :
						if ((icon = GetDiskObject(appname)))
							break;

					case 0  :
						icon = GetDiskObject("ENV:Sys/def_SDL");
						break;
				}

				hidden->DiskObj = icon;
			}

			if (icon)
			{
				CONST_STRPTR name;

				icon->do_CurrentX = NO_ICON_POSITION;
				icon->do_CurrentY = NO_ICON_POSITION;
				icon->do_Type     = 0;

				name	= this->wm_icon && *this->wm_icon ? this->wm_icon : this->wm_title && *this->wm_title ? this->wm_title : "SDL";

				D(bug("[SDL] AddAppIconA(), name = [%s]\n", (IPTR)name));

				hidden->AppIcon = AddAppIconA(0, 0, (char *)name, hidden->WorkbenchPort, 0, icon, NULL);

				if (hidden->AppIcon)
				{
					HideWindow(SDL_Window);
					this->hidden->window_active = 0;
					SDL_PrivateAppActive(0, SDL_APPACTIVE);
				}
			}
		}
	}
#endif // HideWindow unsupported
	return 1;
}

void CGX_UniconifyWindow(_THIS)
{
#if 0
	if (SDL_Window)
	{
		if (currently_fullscreen)
		{
			ScreenToFront(SDL_Display);
		}
		else if (this->hidden->AppIcon)
		{
			struct Message *msg;

			RemoveAppIcon(this->hidden->AppIcon);

			this->hidden->AppIcon = NULL;

			while ((msg = GetMsg(this->hidden->WorkbenchPort)) != NULL)
				ReplyMsg(msg);

			ShowWindow(SDL_Window);
			this->hidden->window_active = 1;
			SDL_PrivateAppActive(1, SDL_APPACTIVE);
		}
	}
#endif // ShowWindow unsupported
}

extern struct InputEvent *InputHandler (struct InputEvent *event, SDL_VideoDevice *this);
static struct Interrupt	Handler		= { { NULL, NULL, 0, 55, "WarpOS SDL Input Lock" }, NULL, (VOID (*)())&InputHandler };
static struct IOStdReq *ioreq 		= NULL;

static void CGX_LockInput(_THIS)
{
   if (ioreq == NULL)
   {
	   struct IOStdReq *req;
	   struct MsgPort  *port;

	   port = CreateMsgPort();
	   req = (struct IOStdReq *)CreateIORequest(port, sizeof(*req));

	   if (req)
	   {
		   if (OpenDevice("input.device", 0, (struct IORequest *)req, 0) == 0)
		   {
			   ioreq   = req;

			   // Lock mouse

			   this->hidden->grabbing_input = 1;

			   Handler.is_Data = this;
			   req->io_Command = IND_ADDHANDLER;
			   req->io_Data = &Handler;
			   DoIO((struct IORequest *)req);
			   return;
		   }
	   }
	   DeleteIORequest(req);
	   DeleteMsgPort(port);
   }
}


static void CGX_UnlockInput(_THIS)
{
   this->hidden->grabbing_input = 0;
   this->hidden->should_grab_input = 0;

   if (ioreq)
   {
	   struct IOStdReq *req	   = ioreq;
	   struct MsgPort  *port   = req->io_Message.mn_ReplyPort;

	   ioreq = NULL;

	   req->io_Command = IND_REMHANDLER;
	   DoIO((struct IORequest *)req);

	   CloseDevice((struct IORequest *)req);
	   DeleteIORequest((struct IORequest *)req);
	   DeleteMsgPort(port);
   }
}

SDL_GrabMode CGX_GrabInputNoLock(_THIS, SDL_GrabMode mode)
{
	D(bug("CGX_GrabInputNoLock()\n"));

	if ( this->screen == NULL ) {
		return(SDL_GRAB_OFF);
	}
	if ( ! SDL_Window ) {
		return(mode);	/* Will be set later on mode switch */
	}
	if ( mode == SDL_GRAB_OFF )
	{
		CGX_UnlockInput(this);
	} else {
		if ( this->screen->flags & SDL_FULLSCREEN ) {
			/* Unbind the mouse from the fullscreen window */
		}
		else
		{
			CGX_LockInput(this);
		}

		/* Try to grab the mouse */
	}

	return(mode);
}

SDL_GrabMode CGX_GrabInput(_THIS, SDL_GrabMode mode)
{
	D(bug("CGX_GrabInput()\n"));

	SDL_Lock_EventThread();
	mode = CGX_GrabInputNoLock(this, mode);
	SDL_Unlock_EventThread();

	return(mode);
}

int CGX_GetWMInfo(_THIS, SDL_SysWMinfo *info)
{
	if ( info->version.major <= SDL_MAJOR_VERSION ) {
		return(1);
	} else {
		SDL_SetError("Application not compiled with SDL %d.%d\n",
					SDL_MAJOR_VERSION, SDL_MINOR_VERSION);
		return(-1);
	}
}
