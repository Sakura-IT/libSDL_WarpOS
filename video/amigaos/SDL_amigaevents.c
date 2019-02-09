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

#pragma pack(push, 2)
#include <proto/dos.h>
#include <proto/keymap.h>
#include <powerpc/powerpc.h>
#pragma pack(pop)

/* Handle the event stream, converting Amiga events into SDL events */
#include "SDL.h"

#include "SDL_syswm.h"
#include "../../events/SDL_sysevents.h"
#include "../SDL_sysvideo.h"
#include "../../events/SDL_events_c.h"
#include "SDL_cgxvideo.h"
#include "SDL_cgxmodes_c.h"
#include "SDL_cgximage_c.h"
#include "SDL_cgxwm_c.h"

#include "SDL_amigaevents_c.h"
#include "SDL_amigamouse_c.h"

/* The translation tables from an Amiga keysym to a SDL keysym */
static SDLKey MISC_keymap[256];
SDL_keysym *amiga_TranslateKey(int code, int qual, SDL_keysym *keysym);
struct IOStdReq *ConReq=NULL;
struct MsgPort *ConPort=NULL;

static int amiga_GetButton(int code)
{
	switch(code)
	{
		case IECODE_MBUTTON:
			return SDL_BUTTON_MIDDLE;
		case IECODE_RBUTTON:
			return SDL_BUTTON_RIGHT;
		default:
			return SDL_BUTTON_LEFT;
	}
}

static int amiga_DispatchEvent(_THIS, struct MsgStruct event)
{
	int class = event.Class;
	int code = event.Code;
	int qualifier = event.Qualifier;
	int mousex = event.MouseX;
	int mousey = event.MouseY;
	int posted;

	posted = 0;

	switch (class) {
	
		 /* Gaining mouse coverage? */
		 case IDCMP_ACTIVEWINDOW:
			this->hidden->window_active = 1;
			this->hidden->oldkey = 0;
			SDL_PrivateAppActive(1, SDL_APPINPUTFOCUS);	// SDL_APPMOUSEFOCUS

			if (this->hidden->should_grab_input)
			{
				CGX_GrabInputNoLock(this, SDL_GRAB_ON);
			}

			break;

		 /* Losing mouse coverage? */
		 case IDCMP_INACTIVEWINDOW:
			this->hidden->window_active = 0;
			this->hidden->grabbing_input = 0;
			SDL_PrivateAppActive(0, SDL_APPINPUTFOCUS);

			if (this->hidden->grabbing_input)
			{
				CGX_GrabInputNoLock(this, SDL_GRAB_OFF);
				this->hidden->should_grab_input = 1;
			}

			break;

	    /* Mouse motion? */
	    case IDCMP_MOUSEMOVE:
            		if ( SDL_VideoSurface )
			{
				/* ... for relative mouse coords */

				if (mouse_relative == 1)
				{
					SDL_PrivateMouseMotion(0, mouse_relative, mousex, mousey);
				}
				else
				{
					posted = SDL_PrivateMouseMotion(0, mouse_relative,
						mousex-SDL_Window->BorderLeft,
						mousey-SDL_Window->BorderTop);
				}
			}
	    		break;

	     case IDCMP_MOUSEBUTTONS:
			/* Mouse button press? */

			if(!(code&IECODE_UP_PREFIX))
			{
				SDL_PrivateMouseButton(SDL_PRESSED, amiga_GetButton(code), 0, 0);
			}
		 		/* Mouse button release? */
			else
			{
				code&=~IECODE_UP_PREFIX;
				SDL_PrivateMouseButton(SDL_RELEASED, amiga_GetButton(code), 0, 0);
			}
			break;
		   
	    case IDCMP_RAWKEY:

		    /* Key press? */
			if ((qualifier && IEQUALIFIER_LALT) && (qualifier && IEQUALIFIER_CONTROL) && (code == 0x25))
			{
				extern int skipframe,toggle;
				
				if (skipframe==0)
					{skipframe = 1; toggle=1;}
				else
                 			{skipframe = 0; toggle=0;}
				
			}
			switch (code)
			{
				case 0x200	:
				{
					SDL_PrivateMouseButton(SDL_PRESSED, SDL_BUTTON_LEFT, 0, 0);
					return(posted);

				}
				case 0x201	:
				{
					SDL_PrivateMouseButton(SDL_RELEASED, SDL_BUTTON_LEFT, 0, 0);
					return(posted);

				}
				case 0x202	:
				{
					SDL_PrivateMouseButton(SDL_PRESSED, SDL_BUTTON_RIGHT, 0, 0);
					return(posted);

				}
				case 0x203	:
				{
					SDL_PrivateMouseButton(SDL_RELEASED, SDL_BUTTON_RIGHT, 0, 0);
					return(posted);

				}
				case 0x7a	:
				case 0x7b	:
				{
					//if(!(code & IECODE_UP_PREFIX))
					{
						SDL_PrivateMouseButton(SDL_PRESSED,
							code == 0x7a ? SDL_BUTTON_WHEELUP : SDL_BUTTON_WHEELDOWN, 0, 0);
					}
					//else
					{
						//code	&= ~IECODE_UP_PREFIX;
						SDL_PrivateMouseButton(SDL_RELEASED,
							code == 0x7a ? SDL_BUTTON_WHEELUP : SDL_BUTTON_WHEELDOWN, 0, 0);
					}
					break;
				}
                	}
             		if( !(code&IECODE_UP_PREFIX) )
		    	{
				SDL_keysym keysym;
				posted = SDL_PrivateKeyboard(SDL_PRESSED,
				amiga_TranslateKey(code, qualifier, &keysym));
		    	}
		    	else
		    	{
	    		/* Key release? */

				SDL_keysym keysym;
				code&=~IECODE_UP_PREFIX;

			/* Check to see if this is a repeated key */
/*				if ( ! X11_KeyRepeat(SDL_Display, &xevent) )  */

				posted = SDL_PrivateKeyboard(SDL_RELEASED,
				amiga_TranslateKey(code, qualifier, &keysym));
					
				if (SDL_TranslateUNICODE)  // fix when press a key and release qualifier before key.key is repeat endless without that fix
				{
					posted = SDL_PrivateKeyboard(SDL_RELEASED,
					amiga_TranslateKey(code,IEQUALIFIER_RSHIFT, &keysym));	
					posted = SDL_PrivateKeyboard(SDL_RELEASED,
					amiga_TranslateKey(code,IEQUALIFIER_RALT, &keysym));
              				posted = SDL_PrivateKeyboard(SDL_RELEASED,
					amiga_TranslateKey(code,IEQUALIFIER_CONTROL, &keysym));
					posted = SDL_PrivateKeyboard(SDL_RELEASED,
					amiga_TranslateKey(code, 0, &keysym));
         			}	
		    	}
		    	break;
   // code from powersdl (not work when press a key and release qualifier before key.key is repeat endless




//					{				{
//						SDL_keysym keysym;
//						amiga_TranslateKey(code & ~IECODE_UP_PREFIX, qualifier, &keysym);
//
//						if (code > 0x7f || !this->hidden->oldkey || this->hidden->oldkey != keysym.sym)
//						{
//							this->hidden->oldkey = code > 0x7f ? 0 : keysym.sym;
//							SDL_PrivateKeyboard(code > 0x7f ? SDL_RELEASED : SDL_PRESSED, &keysym);
//						}
//						this->hidden->oldqual = qualifier;
//					}
				
			break;

	    /* Have we been resized? */
	    case IDCMP_NEWSIZE:
			SDL_PrivateResize(SDL_Window->Width-SDL_Window->BorderLeft-SDL_Window->BorderRight,
		                  SDL_Window->Height-SDL_Window->BorderTop-SDL_Window->BorderBottom);

			break;

	    /* Have we been requested to quit? */
	    case IDCMP_CLOSEWINDOW:
			posted = SDL_PrivateQuit();
			break;

	    /* Do we need to refresh ourselves? */

	    default: {
		/* Only post the event if we're watching for it */
			if ( SDL_ProcessEvents[SDL_SYSWMEVENT] == SDL_ENABLE ) {
				SDL_SysWMmsg wmmsg;
				SDL_VERSION(&wmmsg.version);
				posted = SDL_PrivateSysWMEvent(&wmmsg);
			}
	    }
	    break;
	}

	return(posted);
}

int Sys_GetEvents(void *port,void *msgarray,int arraysize)
{
     extern int GetMessages68k();
     struct PPCArgs args;

     args.PP_Code = (APTR)GetMessages68k;
     args.PP_Offset = 0;
     args.PP_Flags = 0;
     args.PP_Stack = NULL;
     args.PP_StackSize = 0;
     args.PP_Regs[PPREG_A0] = (ULONG)msgarray;
     args.PP_Regs[PPREG_A1] = (ULONG)port;
     args.PP_Regs[PPREG_D0] = arraysize;

     Run68K(&args);
     return args.PP_Regs[PPREG_D0];
}

void amiga_PumpEvents(_THIS)
{
	int i;
	struct MsgStruct events[50];
	if ((!SDL_Window) || (!SDL_Window->UserPort))
		return;

	int messages = Sys_GetEvents(SDL_Window->UserPort, events, 50);

        for (i=0; i<messages; i++)
        {
		amiga_DispatchEvent(this, events[i]);
	}
}

void amiga_InitOSKeymap(_THIS)
{
	int i;

	/* Map the miscellaneous keys */
	for ( i=0; i<SDL_TABLESIZE(MISC_keymap); ++i )
		MISC_keymap[i] = SDLK_UNKNOWN;

	/* These X keysyms have 0xFF as the high byte */
	MISC_keymap[65] = SDLK_BACKSPACE;
	MISC_keymap[66] = SDLK_TAB;
	MISC_keymap[70] = SDLK_CLEAR;
	MISC_keymap[70] = SDLK_DELETE;
	MISC_keymap[68] = SDLK_RETURN;
	MISC_keymap[69] = SDLK_ESCAPE;
	MISC_keymap[15] = SDLK_KP0;		/* Keypad 0-9 */
	MISC_keymap[29] = SDLK_KP1;
	MISC_keymap[30] = SDLK_KP2;
	MISC_keymap[31] = SDLK_KP3;
	MISC_keymap[45] = SDLK_KP4;
	MISC_keymap[46] = SDLK_KP5;
	MISC_keymap[47] = SDLK_KP6;
	MISC_keymap[61] = SDLK_KP7;
	MISC_keymap[62] = SDLK_KP8;
	MISC_keymap[63] = SDLK_KP9;
	MISC_keymap[60] = SDLK_KP_PERIOD;
	MISC_keymap[92] = SDLK_KP_DIVIDE;
	MISC_keymap[93] = SDLK_KP_MULTIPLY;
	MISC_keymap[74] = SDLK_KP_MINUS;
	MISC_keymap[94] = SDLK_KP_PLUS;
	MISC_keymap[67] = SDLK_KP_ENTER;

	MISC_keymap[76] = SDLK_UP;
	MISC_keymap[77] = SDLK_DOWN;
	MISC_keymap[78] = SDLK_RIGHT;
	MISC_keymap[79] = SDLK_LEFT;

	MISC_keymap[71]  = SDLK_INSERT;
	MISC_keymap[112] = SDLK_HOME;
	MISC_keymap[113] = SDLK_END;
	MISC_keymap[108] = SDLK_PRINT;

	MISC_keymap[90] = SDLK_PAGEUP;
	MISC_keymap[91] = SDLK_PAGEDOWN;
	MISC_keymap[72] = SDLK_PAGEUP;		/* added by LouiSe */
	MISC_keymap[73] = SDLK_PAGEDOWN;	/* added by LouiSe */

	MISC_keymap[80] = SDLK_F1;
	MISC_keymap[81] = SDLK_F2;
	MISC_keymap[82] = SDLK_F3;
	MISC_keymap[83] = SDLK_F4;
	MISC_keymap[84] = SDLK_F5;
	MISC_keymap[85] = SDLK_F6;
	MISC_keymap[86] = SDLK_F7;
	MISC_keymap[87] = SDLK_F8;
	MISC_keymap[88] = SDLK_F9;
	MISC_keymap[89] = SDLK_F10;
	MISC_keymap[75] = SDLK_F11;
	MISC_keymap[111] = SDLK_F12;

	MISC_keymap[95] = SDLK_HELP;
	MISC_keymap[96] = SDLK_LSHIFT;
	MISC_keymap[97] = SDLK_RSHIFT;
	MISC_keymap[98] = SDLK_CAPSLOCK;
	MISC_keymap[99] = SDLK_LCTRL;
	MISC_keymap[99] = SDLK_LCTRL;
	MISC_keymap[100] = SDLK_LALT;
	MISC_keymap[101] = SDLK_RALT;
	MISC_keymap[102] = SDLK_LSUPER; /* Left "Windows" */
	MISC_keymap[103] = SDLK_RSUPER; /* Right "Windows */
}

struct Library *KeymapBase;
SDL_keysym *amiga_TranslateKey(int code, int qual, SDL_keysym *keysym)
{
	ULONG key, unicode;
	WORD actual = -1;
	SDLMod	mod;

	D(bug("[SDL] amiga_TranslateKey()\n"));

	/* Get the raw keyboard scancode */
   
		if(!KeymapBase)
		{
			KeymapBase=OpenLibrary("keymap.library", 0L);
		}
	keysym->scancode = code;
	keysym->sym = MISC_keymap[code];
	key = 0;

	/* Get the translated SDL virtual keysym */
	if (keysym->sym == SDLK_UNKNOWN)
	{
		struct InputEvent	ie;
		UBYTE	buffer[4];
        
		//qual &= ~(IEQUALIFIER_CONTROL|IEQUALIFIER_LALT|IEQUALIFIER_RALT);
        
		ie.ie_Class				= IECLASS_RAWKEY;
		ie.ie_SubClass			= 0;
		ie.ie_Code				= code;
		ie.ie_Qualifier		  = SDL_TranslateUNICODE ? qual : qual & ~(IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT|IEQUALIFIER_CAPSLOCK | IEQUALIFIER_CONTROL|IEQUALIFIER_LALT|IEQUALIFIER_RALT);
		//ie.ie_Qualifier		= qual;// & ~(IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT|IEQUALIFIER_CAPSLOCK);
		ie.ie_EventAddress	= NULL;
       
		actual = MapRawKey(&ie, buffer, 4, NULL);

		if (actual == 1)
		{
			key = *buffer;
			keysym->sym = *buffer;
		}
	}

	mod	= KMOD_NONE;

	if (qual & IEQUALIFIER_LSHIFT)
		mod |= KMOD_LSHIFT;
	if (qual & IEQUALIFIER_RSHIFT)
		mod |= KMOD_RSHIFT;
	if (qual & IEQUALIFIER_CAPSLOCK)
		mod |= KMOD_CAPS;
	if (qual & IEQUALIFIER_CONTROL)
		mod |= KMOD_LCTRL | KMOD_RCTRL;
	if (qual & IEQUALIFIER_LALT)
		mod |= KMOD_LALT;
	if (qual & IEQUALIFIER_RALT)
		mod |= KMOD_RALT;
	if (qual & IEQUALIFIER_LCOMMAND)
		mod |= KMOD_LMETA;
	if (qual & IEQUALIFIER_RCOMMAND)
		mod |= KMOD_RMETA;
	if (qual & IEQUALIFIER_NUMERICPAD)
		mod |= KMOD_NUM;

	keysym->mod = mod;

	/* unicode support */
	unicode = 0;

	if (SDL_TranslateUNICODE) 
	{
		unicode = keysym->sym;
		if (unicode > 255)
			unicode = 0;
	}

	keysym->unicode = unicode;
	
	return(keysym);
} 
