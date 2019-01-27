/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2009 Sam Lantinga

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

    Additional (MorphOS) code by LouiSe@louise.amiga.hu

*/
#include "SDL_config.h"

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id$";
#endif

#include "SDL_error.h"
#include "SDL_mouse.h"
#include "SDL_amigamouse_c.h"
#include "../SDL_cursor_c.h"
#include "../../events/SDL_events_c.h"

//#include "mydebug.h"

/* The implementation dependent data for the window manager cursor */

struct WMcursor
{
	UWORD *image;
	WORD	width, height;
	WORD	offx, offy;
};

void amiga_FreeWMCursor(_THIS, WMcursor *cursor)
{
	struct SDL_PrivateVideoData *hidden = this->hidden;

	D(bug("[SDL] amiga_FreeWMCursor()\n"));

	if (cursor)
	{
		hidden->WindowCursor = NULL;

		if (SDL_Window)
			ClearPointer(SDL_Window);

		if (cursor->image)
			free(cursor->image);

		free(cursor);
	}
}

WMcursor *amiga_CreateWMCursor(_THIS, Uint8 *data, Uint8 *mask, int w, int h, int hot_x, int hot_y)
{
	struct WMcursor *cursor = NULL;

	if (w <= 16 && (cursor = malloc(sizeof(*cursor))))
	{
		cursor->width  = w;
		cursor->height = h;
		cursor->offx   = -hot_x;
		cursor->offy   = -hot_y;

		if ((cursor->image = malloc(h * sizeof(ULONG) + 8)))
		{
			ULONG y, *p;

			p = (ULONG *)(cursor->image + 2);

			for (y = 0; y < h; y++)
			{
				if (w <= 8)
				{
					*p++ = (*mask++) << 24 | (*data++) << 8;
				}
				else
				{
					*p++ = mask[0] << 24 | mask[1] << 16 | data[0] << 8 | data[1];
					data += 2;
					mask += 2;
				}
			}
		}
		else
		{
			free(cursor);
			cursor = NULL;
		}
	}

	D(bug("[SDL] amiga_CreateWMCursor() (size %ld/%ld) -> 0x%08.8lx\n", w, h, (IPTR)cursor));

	return cursor;
}

ULONG BlankPointer[1];

int amiga_ShowWMCursor(_THIS, WMcursor *cursor)
{
	struct SDL_PrivateVideoData *hidden = this->hidden;

	D(bug("[SDL] amiga_ShowWMCursor() (%08.8lx, thread 0x%08.8lx)\n", (ULONG)cursor, (ULONG)FindTask(NULL)));

	hidden->WindowCursor = cursor;

	if (SDL_Window)
	{
		SDL_Lock_EventThread();

		if (cursor)
		{
			SetPointer(SDL_Window, cursor->image, cursor->height, cursor->width, cursor->offx, cursor->offy);
		}
		else
		{
			SetPointer(SDL_Window, (UWORD *) BlankPointer, 1, 1, 0, 0);
		}

		SDL_Unlock_EventThread();
	}

	return 1;
}

/* Check to see if we need to enter or leave mouse relative mode */
void amiga_CheckMouseMode(_THIS)
{
	ULONG flags;

	D(bug("[SDL] amiga_CheckMouseMode()\n"));

	/* Minimum IDCMP flags */

	flags = IDCMP_ACTIVEWINDOW|IDCMP_INACTIVEWINDOW|IDCMP_MOUSEMOVE|IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MENUPICK;

	if (!currently_fullscreen)
		flags |= IDCMP_GADGETUP|IDCMP_REFRESHWINDOW|IDCMP_NEWSIZE|IDCMP_CLOSEWINDOW;

	/* If the mouse is hidden and input is grabbed, we use relative mode */

	SDL_Lock_EventThread();
	if ( !(SDL_cursorstate & CURSOR_VISIBLE) && (this->input_grab != SDL_GRAB_OFF) )
	{
		D(bug("relative mouse move\n"));
		mouse_relative = 1;
		ModifyIDCMP(SDL_Window, flags|IDCMP_DELTAMOVE);
	}
	else
	{
		/* non-relative mode */
		D(bug("mouse in non-relative mode\n"));
		mouse_relative = 0;
		ModifyIDCMP(SDL_Window, flags);
	}
	SDL_Unlock_EventThread();
}

void amiga_WarpWMCursor(_THIS, Uint16 x, Uint16 y)
{
/* FIXME: Not implemented */
}
