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

#include "SDL_audio.h"
#include "../SDL_audiomem.h"
#include "../SDL_audio_c.h"
#include "SDL_ahiaudio.h"

#include "../../video/amigaos/SDL_cgxasm.h"

#pragma pack(push,2)
#include <exec/execbase.h>
#include <proto/exec.h>
#include <powerpc/powerpc.h>
#include <powerpc/powerpc_protos.h>
#undef NEWLIST
#define NEWLIST NewListPPC
#pragma pack(pop)

#include "mydebug.h"

#define DataL1LineSize 32

int has_obtained_spec = 0;

/* Audio driver functions */
static int AHI_OpenAudio(_THIS, SDL_AudioSpec *spec);
static void AHI_ThreadInit(_THIS);
static void AHI_WaitAudio(_THIS);
static void AHI_PlayAudio(_THIS);
static Uint8 *AHI_GetAudioBuf(_THIS);
static void AHI_CloseAudio(_THIS);

/* Internal functions */

static Fixed AHI_Volume = 0x10000;

void AHI_Mute(ULONG mute)
{
	AHI_Volume = mute ? 0 : 0x10000;
}

/* Audio driver bootstrap functions */

static int Audio_Available(void)
{
	int ok=0;
	struct MsgPort *p;
	struct AHIRequest *req;

	if(p=CreateMsgPort())
	{
		if(req=(struct AHIRequest *)CreateIORequest(p,sizeof(struct AHIRequest)))
		{
			req->ahir_Version=4;

			if(!OpenDevice(AHINAME,0,(struct IORequest *)req,NULL))
			{
				D(bug("AHI available.\n"));
				ok=1;
				CloseDevice((struct IORequest *)req);
			}
			DeleteIORequest((struct IORequest *)req);
		}
		DeleteMsgPort(p);
	}

	D(if(!ok) bug("AHI not available\n"));
	return ok;
}


static void Sys_Wait68k(struct AHIRequest *req, struct MsgPort *msg){
     extern void Wait68k();
     struct PPCArgs args;
     args.PP_Code = (APTR)Wait68k;
     args.PP_Offset = 0;
     args.PP_Flags = 0;
     args.PP_Stack = NULL;
     args.PP_StackSize = 0;
     args.PP_Regs[PPREG_A0] = (ULONG)req;
     args.PP_Regs[PPREG_A1] = (ULONG)msg;
     Run68K(&args);
}

void static AHI_WaitAudio(_THIS)
{
	struct AHIRequest *req;

	D(bug("[SDL] AHI_WaitAudio()\n"));

	req = &this->hidden->audio_req[this->hidden->current_buffer];

	if (req->ahir_Std.io_Data)
	{
		#if 0

		WaitIO((struct IORequest *)req);
		req->ahir_Std.io_Data = NULL;

		GetMsg(&this->hidden->thread_audio_mp);
		GetMsg(&this->hidden->thread_audio_mp);

		#else

		Sys_Wait68k(req, &this->hidden->thread_audio_mp);
		req->ahir_Std.io_Data = NULL;

		#endif
	}
}

static void AHI_PlayAudio(_THIS)
{
	struct SDL_PrivateAudioData *hidden = this->hidden;
	struct AHIRequest *req;
	APTR   mixbuf;
	ULONG current, current2;
	LONG mixbufsize;

	D(bug("[SDL] AHI_PlayAudio()\n"));

	current  = hidden->current_buffer;
	current2 = current ^ 1;
	req = &hidden->audio_req[current];

	/* Write the audio data out */

	mixbufsize = hidden->size; /* Note: this is an audio buffer size in _bytes_ */
	mixbuf	  = hidden->mixbuf[current];

	req->ahir_Std.io_Data    = mixbuf;
	req->ahir_Std.io_Length  = mixbufsize;
	req->ahir_Std.io_Offset  = 0;
	req->ahir_Std.io_Command = CMD_WRITE;
	req->ahir_Frequency      = hidden->freq;
	req->ahir_Volume         = AHI_Volume;	//0x10000;
	req->ahir_Type           = hidden->type;
	req->ahir_Position       = 0x8000;
	req->ahir_Link           = (hidden->playing ? &hidden->audio_req[current2] : NULL);

	switch (hidden->convert)
	{
		case AHI_CONVERT_8:
		{
			ULONG *buf4 = mixbuf;

			do
			{
				*buf4++ ^= 0x80808080;
				*buf4++ ^= 0x80808080;
				mixbufsize -= 8;
			}
			while (mixbufsize >= 8);
		}
		break;

		case AHI_CONVERT_16:
		{
			WORD *buf = mixbuf;

			do
			{
				buf[0] = buf[0] - 32768;
				buf[1] = buf[1] - 32768;
				buf[2] = buf[2] - 32768;
				buf[3] = buf[3] - 32768;
				buf += 4;
				mixbufsize -= 8;
			}
			while (mixbufsize > 8);
		}
		break;

		case AHI_CONVERT_S16LSB:
			copy_and_swap16(mixbuf, mixbuf, mixbufsize / 2);
			break;

		case AHI_CONVERT_U16LSB:
		{
			UWORD *buf = mixbuf;

			do
			{
				UWORD sample = *buf;
				*buf++ = ((sample >> 8) | (sample << 8)) - 32768;
				*buf++ = ((sample >> 8) | (sample << 8)) - 32768;
				*buf++ = ((sample >> 8) | (sample << 8)) - 32768;
				*buf++ = ((sample >> 8) | (sample << 8)) - 32768;
				mixbufsize -= 8;
			}
			while (mixbufsize > 8);
		}
		break;
	}

	SendIO((struct IORequest *)req);
	hidden->current_buffer = current2;
	hidden->playing = 1;
}

static Uint8 *AHI_GetAudioBuf(_THIS)
{
	D(bug("[SDL] AHI_GetAudioBuf()\n"));
	return(this->hidden->mixbuf[this->hidden->current_buffer]);
}

static void AHI_CloseAudio(_THIS)
{
	struct SDL_PrivateAudioData *hidden = this->hidden;

	D(bug("[SDL] AHI_CloseAudio()\n"));

	if (hidden->have_ahi)
	{
		CloseDevice((struct IORequest *)&hidden->ctrl_req);
	}

	if (hidden->mixbuf[0])
		FreeVecPPC(hidden->mixbuf[0]);

	if (hidden->mixbuf[1])
		FreeVecPPC(hidden->mixbuf[1]);

	hidden->mixbuf[0]  = NULL;
	hidden->mixbuf[1]  = NULL;
	hidden->have_ahi   = 0;
	hidden->playing    = 0;
}

static int AHI_OpenAudio(_THIS, SDL_AudioSpec *spec)
{	
	struct SDL_PrivateAudioData *hidden = this->hidden;
	size_t l1size;

	D(bug("[SDL] AHI_OpenAudio()\n"));

	if (spec->channels > 2)
	{
		D(bug("More than 2 channels not currently supported, forcing conversion...\n"));
		spec->channels = 2;
	}

	D(bug("requested samples is %ld\n", spec->samples));

	/* Fiddle with this to find out working combination with games
	 *
	 * Recommended testing: E-UAE, Bolcatoid
	 */
#ifdef WARPUP
	if (spec->samples < 1024)
		spec->samples = 1024;
#else
	if (has_obtained_spec && spec->samples > 1024)
	{
		spec->samples = spec->freq / 20;

		if (spec->samples < 256)
			spec->samples = 256;

		/* Align to multiples of 8 samples */
		spec->samples = (spec->samples + 0x7) & ~0x7;

		D(bug("[SDL] Changed sample count. New count is %ld.\n", spec->samples));
	}
#endif

	hidden->convert = AHI_NO_CONVERSION;

	/* Determine the audio parameters from the AudioSpec */

	switch (spec->format)
	{
		case AUDIO_U8:
			hidden->convert = AHI_CONVERT_8;
		case AUDIO_S8:
			hidden->bytespersample = 1;
			hidden->type = spec->channels == 1 ? AHIST_M8S : AHIST_S8S;
			break;

		case AUDIO_S16LSB:
		case AUDIO_U16LSB:
		case AUDIO_U16MSB:
			hidden->convert = spec->format == AUDIO_U16MSB ? AHI_CONVERT_16 : spec->format == AUDIO_U16LSB ? AHI_CONVERT_U16LSB : AHI_CONVERT_S16LSB;
		case AUDIO_S16MSB:
			hidden->bytespersample = 2;
			hidden->type = spec->channels == 1 ? AHIST_M16S : AHIST_S16S;
			break;

		default:
			SDL_SetError("Unsupported audio format");
			return(-1);
	}

	/* Update the fragment size as size in bytes */
	SDL_CalculateAudioSpec(spec);

	hidden->ctrl_req.ahir_Std.io_Message.mn_ReplyPort = &hidden->dummy_mp;
	hidden->ctrl_req.ahir_Std.io_Message.mn_Length = sizeof(struct AHIRequest);
	hidden->ctrl_req.ahir_Version = 4;

	if (OpenDevice(AHINAME, 0, (struct IORequest *)&hidden->ctrl_req, NULL) != 0)
	{
		D(bug("[SDL] AHI_OpenAudio() failed!\n"));
		SDL_SetError("Unable to open AHI device! Error code %d.\n", hidden->ctrl_req.ahir_Std.io_Error);
		//DeleteIORequest((struct IORequest *)hidden->ctrl_req);
		return -1;
	}

	hidden->have_ahi = 1;

	/* Set output frequency and size */
	hidden->freq = spec->freq;
	hidden->size = spec->size;

	l1size = DataL1LineSize;

	/* Align size to L1 cache line size */
	hidden->size = (hidden->size + l1size - 1) & ~(l1size - 1);

	/* Allocate mixing buffer aligned to L1 cache line size*/
	hidden->mixbuf[0] = (Uint8 *)AllocVecPPC(hidden->size, MEMF_ANY, l1size);
	hidden->mixbuf[1] = (Uint8 *)AllocVecPPC(hidden->size, MEMF_ANY, l1size);

	if ((hidden->mixbuf[0] == NULL) || (hidden->mixbuf[1] == NULL))
	{
		D(bug("could not alloc audio mixing buffers\n"));
		SDL_SetError("Unable to allocate audio mixing buffers!\n");
		AHI_CloseAudio(this);
		return -1;
	}

	NEWLIST(&hidden->thread_audio_mp.mp_MsgList);
	hidden->thread_audio_mp.mp_Node.ln_Pri	= 60;
	hidden->thread_audio_mp.mp_Node.ln_Type	= NT_MSGPORT;
	hidden->thread_audio_mp.mp_Flags	= PA_SIGNAL;

	hidden->current_buffer			= 0;
	hidden->playing				= 0;

	D(bug("OpenAudio...OK\n"));

	/* We're ready to rock and roll. :-) */

	return 0;
}

static void AHI_WaitDone(_THIS)
{
	struct SDL_PrivateAudioData *hidden = this->hidden;
	int i;

	D(bug("[SDL] AHI_WaitDone()\n"));

	for (i = 0; i < 2; i++)
	{
		struct AHIRequest *req = &hidden->audio_req[i];

		if (req->ahir_Std.io_Data)
		{
			WaitIO((struct IORequest *)req);
			req->ahir_Std.io_Data = NULL;
		}
	}

	NEWLIST(&hidden->thread_audio_mp.mp_MsgList);
}

static void AHI_ThreadInit(_THIS)
{
	struct SDL_PrivateAudioData *hidden = this->hidden;
	struct MsgPort	*mp;
	struct Task	*task;

	D(bug("[SDL] AHI_ThreadInit()\n"));

	mp = &hidden->thread_audio_mp;
	mp->mp_SigTask = task = FindTask(NULL);
	mp->mp_SigBit  = AllocSignal(-1);
	hidden->ctrl_req.ahir_Std.io_Message.mn_ReplyPort = mp;

	CopyMemPPC(&hidden->ctrl_req, &hidden->audio_req[0], sizeof(struct AHIRequest));
	CopyMemPPC(&hidden->ctrl_req, &hidden->audio_req[1], sizeof(struct AHIRequest));

	SetTaskPri(task, 1);
}

static void Audio_DeleteDevice(SDL_AudioDevice *device)
{
	D(bug("[SDL] Audio_DeleteDevice()\n"));
	free(device);
}

static SDL_AudioDevice *Audio_CreateDevice(int devindex)
{
	struct SDL_PrivateAudioData *hidden;
	SDL_AudioDevice *this;

	D(bug("[SDL] Audio_CreateDevice()\n"));

	/* Initialize all variables that we clean on shutdown */
	this = (SDL_AudioDevice *)malloc(sizeof(SDL_AudioDevice) + sizeof(*hidden));

	if (this == NULL)
	{
		SDL_OutOfMemory();
		return(0);
	}

	hidden = (APTR)(((size_t)this) + sizeof(*this));

	memset(this, 0, sizeof(*this) + sizeof(*hidden));

	NEWLIST(&hidden->dummy_mp.mp_MsgList);
	hidden->dummy_mp.mp_Node.ln_Type = NT_MSGPORT;
	hidden->dummy_mp.mp_Flags = PA_IGNORE;

	this->hidden = hidden;

	/* Set the function pointers */
	this->OpenAudio = AHI_OpenAudio;
	this->ThreadInit = AHI_ThreadInit;
	this->WaitAudio = AHI_WaitAudio;
	this->PlayAudio = AHI_PlayAudio;
	this->GetAudioBuf = AHI_GetAudioBuf;
	this->WaitDone = AHI_WaitDone;
	this->CloseAudio = AHI_CloseAudio;

	this->free = Audio_DeleteDevice;

	return this;
}

AudioBootStrap AHI_bootstrap = {
	"AHI",
	"ahi.device SDL audio",
	Audio_Available,
	Audio_CreateDevice
};
