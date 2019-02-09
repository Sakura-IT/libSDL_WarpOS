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

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id$";
#endif

#include "SDL_amigaevents_c.h"
#include <intuition/intuition.h>
#include <exec/avl.h>
#include <inline/exec_protos.h>

int GetMessages68k(__reg("a1") struct MsgPort *port, __reg("a0") struct MsgStruct *msg,	 __reg("d0") int maxmsg)
{
 int i = 0;
 struct IntuiMessage *imsg;

 while ((imsg = (struct IntuiMessage *)GetMsg(port))) {
   if (i < maxmsg) {
     msg[i].Code = imsg->Code;
     msg[i].Class = imsg->Class;
     msg[i].Qualifier = imsg->Qualifier;
     msg[i].MouseX = imsg->MouseX;
     msg[i].MouseY = imsg->MouseY;
     i++;
   }
   ReplyMsg((struct Message *)imsg);
 }
 return i;
}
