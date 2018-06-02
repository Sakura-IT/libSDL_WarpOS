/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997, 1998, 1999, 2000, 2001  Sam Lantinga

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
 
/* StormMesa implementation of SDL OpenGL support */
#include <SDL_config.h> 
#include "SDL_error.h"
#include "SDL_cgxgl_c.h"
#include "SDL_cgxvideo.h"

#pragma pack(2)
#include <mgl/gl.h>
#pragma pack()

extern void *AmiGetGLProc(const char *proc);
#define	GL_ACCUM_RED_BITS                0x0D58
#define	GL_ACCUM_GREEN_BITS              0x0D59
#define	GL_ACCUM_BLUE_BITS               0x0D5A
#define	GL_ACCUM_ALPHA_BITS              0x0D5B
#define	GL_STENCIL_BITS                  0x0D57

/* Init OpenGL */
int CGX_GL_Init(_THIS)
{
#ifdef SDL_VIDEO_OPENGL
	SDL_VideoDevice *video = current_video;
	mini_CurrentContext = NULL;
	MGLInit();
	mglChoosePixelDepth(video->gl_config.depth_size);
	mglChooseNumberOfBuffers((video->gl_config.double_buffer)+1);
	
	if(!currently_fullscreen) {
		mglChooseWindowMode(GL_TRUE);
		if ( SDL_Window ) {
			mini_CurrentContext = mglAttachContext(0,0,SDL_Window->Width,SDL_Window->Height,SDL_Window, 0);
		}
	}

	else {
		mglChooseWindowMode(GL_FALSE);
		if ( SDL_Display ) {
			mglProposeCloseDesktop(GL_TRUE);
			mini_CurrentContext = mglAttachContext(0,0,SDL_Display->Width,SDL_Display->Height,SDL_Window,SDL_Display);
		}
	}

	if ( mini_CurrentContext == NULL ) {
		SDL_SetError("Couldn't create OpenGL context");
		return(-1);
	}
	mglLockMode(MGL_LOCK_SMART);
	this->gl_data->gl_active = 1;
	this->gl_config.driver_loaded = 1;

	return(0);
#else
	SDL_SetError("OpenGL support not configured");
	return(-1);
#endif
}

/* Quit OpenGL */
void CGX_GL_Quit(_THIS)
{
#ifdef SDL_VIDEO_OPENGL
	if ( mini_CurrentContext != NULL ) {
		mglDetachContext();
		mini_CurrentContext = NULL;
		this->gl_data->gl_active = 0;
		this->gl_config.driver_loaded = 0;
	}
	MGLMiniTerm();
#endif
}

/* Attach context to another window */
int CGX_GL_Update(_THIS)
{
	D(bug("[SDL] CGX_GL_Update()\n"));
#ifdef SDL_VIDEO_OPENGL
	if ( mini_CurrentContext != NULL )
	{
		D(bug("... Not Supported by MiniGL!\n"));
	}
	else
	{
		SDL_SetError("Couldn't reinitialize context!");
		return(-1);
	}
	return 0;
#else
	SDL_SetError("OpenGL support not configured");
	return -1;
#endif
}

#ifdef SDL_VIDEO_OPENGL

/* Make the current context active */
int CGX_GL_MakeCurrent(_THIS)
{
	D(bug("[SDL] CGX_GL_MakeCurrent() [0x%08lx]\n", mini_CurrentContext));

	if(mini_CurrentContext == NULL)
		return -1;

	return 0;
}

void CGX_GL_SwapBuffers(_THIS)
{
	D(bug("[SDL] CGX_GL_SwapBuffers()\n"));

	mglSwitchDisplay();
}

int CGX_GL_GetAttribute(_THIS, SDL_GLattr attrib, int* value) {
	GLenum mesa_attrib;
	int val;

	D(bug("[SDL] CGX_GL_GetAttribute()\n"));

	switch(attrib) {
		case SDL_GL_RED_SIZE:
			mesa_attrib = GL_RED_BITS;
			val = 5;
			break;
		case SDL_GL_GREEN_SIZE:
			mesa_attrib = GL_GREEN_BITS;
			val = 6;
			break;
		case SDL_GL_BLUE_SIZE:
			mesa_attrib = GL_BLUE_BITS;
			val = 5;
			break;
		case SDL_GL_ALPHA_SIZE:
			mesa_attrib = GL_ALPHA_BITS;
			val = 0;
			break;
		case SDL_GL_DOUBLEBUFFER:
			mesa_attrib = GL_DOUBLEBUFFER;
			val = 1;
			break;
		case SDL_GL_DEPTH_SIZE:
			mesa_attrib = GL_DEPTH_BITS;
			val = 16; /* or 32? */
			break;
		case SDL_GL_STENCIL_SIZE:
			mesa_attrib = GL_STENCIL_BITS;
			val = 0;
			break;
		case SDL_GL_ACCUM_RED_SIZE:
			mesa_attrib = GL_ACCUM_RED_BITS;
			val = 0;
			break;
		case SDL_GL_ACCUM_GREEN_SIZE:
			mesa_attrib = GL_ACCUM_GREEN_BITS;
			val = 0;
			break;
		case SDL_GL_ACCUM_BLUE_SIZE:
			mesa_attrib = GL_ACCUM_BLUE_BITS;
			val = 0;
			break;
		case SDL_GL_ACCUM_ALPHA_SIZE:
			mesa_attrib = GL_ACCUM_ALPHA_BITS;
			val = 0;
			break;
		default :
			return -1;
	}

	*value = val;
	return 0;
}

void *CGX_GL_GetProcAddress(_THIS, const char *proc) {
	void *func = NULL;

	D(bug("[SDL] CGX_GL_GetProcAddress()\n"));

	func = AmiGetGLProc(proc);

	return func;
}

int CGX_GL_LoadLibrary(_THIS, const char *path) {
	/* Library is always open */
	
	D(bug("[SDL] CGX_GL_LoadLibrary()\n"));
	
	this->gl_config.driver_loaded = 1;

	return 0;
}

#endif /* HAVE_OPENGL */

