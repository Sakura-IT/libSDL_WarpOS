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

#include <SDL_config.h>
#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id$";
#endif

#include "SDL/SDL_config.h"
#include "SDL_syswm.h"
#include "../../events/SDL_events_c.h"
#include "SDL_cgxmodes_c.h"
#include "SDL_cgxwm_c.h"

#ifdef __VBCC__
#include <inline/cybergraphics_protos.h>
#endif
#include <devices/input.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/wb.h>
#include <proto/icon.h>
#include <intuition/intuitionbase.h>

#ifdef __VBCC__
struct InputEvent *InputHandler(__reg("a0") struct InputEvent *event, __reg("a1") SDL_VideoDevice *this) //must be 68k....
#else
static struct InputEvent *InputHandler( struct InputEvent *event, SDL_VideoDevice *this) //must be 68k....
#endif

{
   
   struct InputEvent *ie;

	ie = event;
 
	if (this->hidden->window_active == 0)
		return event;
	if (SDL_Window != this->hidden->IntuiBase->ActiveWindow)
		return event;
   do
   {
	   if (ie->ie_Class == IECLASS_RAWMOUSE)
	   {
		   if (ie->ie_Code == IECODE_LBUTTON)
		   {
			   ie->ie_Class		   = IECLASS_RAWKEY;
			   ie->ie_Code		   = 0x200;
		   }
		   
		   if (ie->ie_Code == (IECODE_LBUTTON | IECODE_UP_PREFIX))
		   {
			   ie->ie_Class		   = IECLASS_RAWKEY;
			   ie->ie_Code		   = 0x201;
		   }

		   else if (ie->ie_Code == IECODE_RBUTTON)
		   {
			   ie->ie_Class		   = IECLASS_RAWKEY;
			   ie->ie_Code		   = 0x202;
		   }
		   else if (ie->ie_Code == (IECODE_RBUTTON | IECODE_UP_PREFIX))
		   {
			   ie->ie_Class		   = IECLASS_RAWKEY;
			   ie->ie_Code		   = 0x203;
		   }
	   }
   }
   while ((ie = ie->ie_NextEvent) != NULL);

   return event;
}

