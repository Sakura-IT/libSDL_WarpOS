/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2010 Sam Lantinga

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


		20040501	Debug lines added
*/
#include "SDL_config.h"

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id$";
#endif

#define USE_INLINE_STDARG

#include <stdlib.h>
#include <string.h>
#include "SDL_error.h"
#include "SDL_video.h"
#include "SDL_cgxyuv_c.h"
#include "../SDL_yuvfuncs.h"

#include "mydebug.h"

#include <cybergraphx/cgxvideo.h>
#include <proto/cgxvideo.h>

#ifndef SRCFMT_YCbCr420
#define SRCFMT_YCbCr420 4
#endif

struct private_yuvhwdata {
	struct Library *CGXVideoBase;
	APTR VHandle;

	/* These are just so we don't have to allocate them separately */
	Uint16 pitches[3];
	Uint8 *planes[3];
	Uint32 cgx_format;
};

/**********************************************************************
	CGX_DeleteOverlay

	Remove overlay but keep structure (iconify/uniconify)
**********************************************************************/

void CGX_DeleteOverlay(_THIS, SDL_Overlay *overlay)
{
	struct private_yuvhwdata *hwdata = overlay->hwdata;
	struct Library	*CGXVideoBase	= hwdata->CGXVideoBase;

	if (hwdata->VHandle)
	{
		if ( this->hidden->overlay_attached )
			DetachVLayer(hwdata->VHandle);

		DeleteVLayerHandle(hwdata->VHandle);
		hwdata->VHandle = NULL;
	}
}

/**********************************************************************
	CGX_RecreateOverlay

	Restore overlay
**********************************************************************/

void CGX_RecreateOverlay(_THIS, SDL_Overlay *overlay)
{
	struct private_yuvhwdata *hwdata = overlay->hwdata;
	struct Library	*CGXVideoBase	= hwdata->CGXVideoBase;

	hwdata->VHandle	= CreateVLayerHandleTags(SDL_Display,
		VOA_SrcType, hwdata->cgx_format,
		VOA_SrcWidth, overlay->w,
		VOA_SrcHeight, overlay->h,
		VOA_UseColorKey, TRUE,
		TAG_DONE);

	if (hwdata->VHandle && this->hidden->overlay_attached)
	{
		SDL_Rect *dstrect;
		ULONG right, left, top, bottom;

		dstrect = &this->hidden->overlay_rect;

		left = dstrect->x;
		top = dstrect->y;

		right = (SDL_Window->Width - SDL_Window->BorderLeft - SDL_Window->BorderRight);
		if (right > (dstrect->w + dstrect->x))
			right -= dstrect->w + dstrect->x;
		else
			right = 0;

		bottom = (SDL_Window->Height - SDL_Window->BorderTop - SDL_Window->BorderBottom);
		if (bottom > (dstrect->h + dstrect->y))
			bottom -= dstrect->h + dstrect->y;
		else
			bottom = 0;

		if (AttachVLayerTags(hwdata->VHandle, SDL_Window, VOA_LeftIndent, left, VOA_RightIndent, right, VOA_TopIndent, top, VOA_BottomIndent, bottom, TAG_DONE) == 0)
		{
			this->hidden->overlay_colorkey = GetVLayerAttr(hwdata->VHandle, VOA_ColorKey);
			this->hidden->overlay_left  = left;
			this->hidden->overlay_top   = top;
			this->hidden->overlay_right = right;
			this->hidden->overlay_bottom = bottom;
		}
	}
}

int CGX_LockYUVOverlay(_THIS, SDL_Overlay *overlay)
{
	struct private_yuvhwdata *hwdata = overlay->hwdata;
	struct Library	*CGXVideoBase	= hwdata->CGXVideoBase;

	if (LockVLayer(hwdata->VHandle))
	{
		APTR pixels = (APTR)GetVLayerAttr(hwdata->VHandle, VOA_BaseAddress);

		switch (overlay->format)
		{
			case SDL_YV12_OVERLAY:
				overlay->pixels[0] = pixels;
				overlay->pixels[2] = overlay->pixels[0] + overlay->pitches[0] * overlay->h;
				overlay->pixels[1] = overlay->pixels[2] + overlay->pitches[2] * overlay->h / 2;
				break;
			case SDL_IYUV_OVERLAY:
				overlay->pixels[0] = pixels;
				overlay->pixels[1] = overlay->pixels[0] + overlay->pitches[0] * overlay->h;
				overlay->pixels[2] = overlay->pixels[1] + overlay->pitches[1] * overlay->h / 2;
				break;
			case SDL_YUY2_OVERLAY:
			case SDL_UYVY_OVERLAY:
			case SDL_YVYU_OVERLAY:
				overlay->pixels[0] = pixels;
				break;
		}

		return 0;
	}

	return -1;
}

void CGX_UnlockYUVOverlay(_THIS, SDL_Overlay *overlay)
{
	if (overlay->pixels[0])
	{
		struct private_yuvhwdata *hwdata = overlay->hwdata;
		struct Library	*CGXVideoBase	= hwdata->CGXVideoBase;

		overlay->pixels[0] = NULL;
		overlay->pixels[1] = NULL;
		overlay->pixels[2] = NULL;
		UnlockVLayer(hwdata->VHandle);
	}
}

int CGX_DisplayYUVOverlay(_THIS, SDL_Overlay *overlay, SDL_Rect *srcrect, SDL_Rect *dstrect)
{
	struct private_yuvhwdata *hwdata = overlay->hwdata;
	struct Library	*CGXVideoBase	= hwdata->CGXVideoBase;
	ULONG right,left,top,bottom;

	left = dstrect->x;
	top = dstrect->y;

	right = (SDL_Window->Width - SDL_Window->BorderLeft - SDL_Window->BorderRight);
	if (right > (dstrect->w + dstrect->x))
		right -= dstrect->w + dstrect->x;
	else
		right = 0;

	bottom = (SDL_Window->Height - SDL_Window->BorderTop - SDL_Window->BorderBottom);
	if (bottom > (dstrect->h + dstrect->y))
		bottom -= dstrect->h + dstrect->y;
	else
		bottom = 0;

	this->hidden->overlay_rect = *dstrect;

	if (this->hidden->overlay_attached)
	{
		if (this->hidden->overlay_left == left || this->hidden->overlay_right == right || this->hidden->overlay_top == top || this->hidden->overlay_bottom == bottom)
			return 0;

		SetVLayerAttrTags(hwdata->VHandle, VOA_LeftIndent, left, VOA_RightIndent, right, VOA_TopIndent, top, VOA_BottomIndent, bottom, TAG_DONE);
	}
	else
	{
		if (AttachVLayerTags(hwdata->VHandle, SDL_Window, VOA_LeftIndent, left, VOA_RightIndent, right, VOA_TopIndent, top, VOA_BottomIndent, bottom, TAG_DONE) == 0)
		{
			this->hidden->overlay_colorkey = GetVLayerAttr(hwdata->VHandle, VOA_ColorKey);
			this->hidden->overlay_attached = 1;
		}
		else
		{
			return -1;
		}
	}

	this->hidden->overlay_left  = left;
	this->hidden->overlay_top   = top;
	this->hidden->overlay_right = right;
	this->hidden->overlay_bottom = bottom;

	return 0;
}

void CGX_FreeYUVOverlay(_THIS, SDL_Overlay *overlay)
{
	struct private_yuvhwdata *hwdata = overlay->hwdata;

	this->hidden->overlay = NULL;

	if ( hwdata )
	{
		if (hwdata->VHandle)
			CGX_DeleteOverlay(this, overlay);

		this->hidden->overlay_attached = 0;

		CloseLibrary(hwdata->CGXVideoBase);
		free(hwdata);
	}
}

/* The functions used to manipulate software video overlays */
static const struct private_yuvhwfuncs cgx_yuvfuncs =
{
	CGX_LockYUVOverlay,
	CGX_UnlockYUVOverlay,
	CGX_DisplayYUVOverlay,
	CGX_FreeYUVOverlay
};

SDL_Overlay *CGX_CreateYUVOverlay(_THIS, int width, int height, Uint32 format, SDL_Surface *display)
{
	struct Library	*CGXVideoBase;
	SDL_Overlay *overlay;
	struct private_yuvhwdata *hwdata;
	ULONG srcfmt;

	/* Disabled SDL_YUY2_OVERLAY
	 *
	 * Would need byte swapping for SRCFMT_YCbCr16 format (itix)
	 */

	if ( format != SDL_YUY2_OVERLAY && format != SDL_YV12_OVERLAY && format != SDL_IYUV_OVERLAY ) {
//	if ( format != SDL_YV12_OVERLAY && format != SDL_IYUV_OVERLAY ) {
		SDL_SetError("No support for requested YUV format");
		return(NULL);
	}

	/* Create the overlay structure */
	overlay = (SDL_Overlay *)malloc(sizeof *overlay);
	if ( overlay == NULL ) {
		SDL_OutOfMemory();
		return(NULL);
	}
	memset(overlay, 0, (sizeof *overlay));

	/* Fill in the basic members */
	overlay->format = format;
	overlay->w = width;
	overlay->h = height;
	overlay->hw_overlay = 1;

	/* Set up the YUV surface function structure */
	overlay->hwfuncs = (struct private_yuvhwfuncs *)&cgx_yuvfuncs;

	/* Create the pixel data and lookup tables */
	hwdata = (struct private_yuvhwdata *)malloc(sizeof *hwdata);
	overlay->hwdata = hwdata;
	if ( hwdata == NULL ) {
		SDL_OutOfMemory();
		SDL_FreeYUVOverlay(overlay);
		return(NULL);
	}
	memset(hwdata, 0, (sizeof *hwdata));

	overlay->pitches = hwdata->pitches;
	overlay->pixels = hwdata->planes;

	if ((CGXVideoBase = OpenLibrary("cgxvideo.library", 41)) == NULL)
	{
		SDL_SetError("No cgxvideo.library V41+ found!");
		SDL_FreeYUVOverlay(overlay);
		return NULL;
	}

	hwdata->CGXVideoBase	= CGXVideoBase;

	switch (format)
	{
		case SDL_YV12_OVERLAY:
		case SDL_IYUV_OVERLAY:
			srcfmt = SRCFMT_YCbCr420;	/* We currently only support these.. */
			break;
		case SDL_YUY2_OVERLAY:
			srcfmt = SRCFMT_YCbCr16;	/* ..and this (anything else is rejected at the beginning of func) */
			break;
		case SDL_UYVY_OVERLAY:
		case SDL_YVYU_OVERLAY:
		default:
			srcfmt = 0xffffffff;		/* just to avoid warning */
			break;
	}

	hwdata->cgx_format = srcfmt;

	hwdata->VHandle	= CreateVLayerHandleTags(SDL_Display,
		VOA_SrcType, srcfmt,
		VOA_SrcWidth, width,
		VOA_SrcHeight, height,
		VOA_UseColorKey, TRUE,
		TAG_DONE);

	if (hwdata->VHandle == NULL)
	{
		SDL_OutOfMemory();
		SDL_FreeYUVOverlay(overlay);
		return(NULL);
	}

	width = GetVLayerAttr(hwdata->VHandle, VOA_Modulo);

	switch (format)
	{
		case SDL_YV12_OVERLAY:
		case SDL_IYUV_OVERLAY:
			overlay->pitches[0] = width/2;
			overlay->pitches[1] = width/4;
			overlay->pitches[2] = width/4;
			overlay->planes = 3;
			break;
		case SDL_YUY2_OVERLAY:
		case SDL_UYVY_OVERLAY:
		case SDL_YVYU_OVERLAY:
			overlay->pitches[0] = width*2;
			overlay->planes = 1;
			break;
	}

	this->hidden->overlay = overlay;

	return(overlay);
}
