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
*/
#include "SDL_config.h"

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id$";
#endif

/* Stub functions for system-level CD-ROM audio control */

#include	<stdio.h>

#include "SDL_cdrom.h"
#include "../SDL_syscdrom.h"

#pragma pack(2)
#include	<dos/dosextens.h>
#include	<dos/filehandler.h>
#include	<devices/scsidisk.h>
#include	<proto/dos.h>
#include	<proto/exec.h>
#include	<powerpc/powerpc_protos.h>
#pragma pack()

	/* Just large enought to hold TOC */

#define	TOC_SIZE				804
#define	SCSI_INBUF_SIZE	TOC_SIZE
#define	SCSI_SENSE_SIZE	252
#define MAX_DRIVES 32

struct MyCDROM
{
	char	dosname[20];
	char	devname[108];
	ULONG	unit;
	ULONG	unitflags;

	/* Filled in when this unit is opened	*/

	struct IOStdReq	*req;
	struct SCSICmd		scsicmd;
	UWORD	*inbuf;
	UBYTE	*sensebuf;
	struct MsgPort	*port;
};

struct MyCDROM *CDList[MAX_DRIVES];

struct TrackInfo
{
	UBYTE	Pad1, Flags, TrackNumber, Pad2;
	ULONG	Address;
};

struct CD_TOC
{
	UWORD	Length;
	UBYTE	FirstTrack;
	UBYTE	LastTrack;
	struct TrackInfo	TOCData[100];
};



/**********************************************************************
	FindDup

	Return 1 if duplicate
**********************************************************************/

static ULONG FindDup(struct MyCDROM **cdlist, CONST_STRPTR devname, ULONG unit)
{
	struct MyCDROM	*entry;
	ULONG	i;

	for (i = 0; i < MAX_DRIVES; i++)
	{
		entry	= (struct MyCDROM *)cdlist[i];

		if (entry == NULL)
			break;

		if (entry->unit == unit && strcmp(entry->devname, devname) == 0)
			return 1;
	}

	return 0;
}

/**********************************************************************
	SendCMD
**********************************************************************/

static LONG SendCMD(SDL_CD *cdrom, const UBYTE *cmd, int cmdlen)
{
	struct MyCDROM	*entry	= CDList[cdrom->id];

	entry->scsicmd.scsi_Data			= entry->inbuf;
	entry->scsicmd.scsi_Length			= SCSI_INBUF_SIZE;
	entry->scsicmd.scsi_SenseActual	= 0;
	entry->scsicmd.scsi_SenseData		= (UBYTE *)entry->sensebuf;
	entry->scsicmd.scsi_SenseLength	= SCSI_SENSE_SIZE;
	entry->scsicmd.scsi_CmdActual		= 0;
	entry->scsicmd.scsi_Command		= (UBYTE *)cmd;
	entry->scsicmd.scsi_CmdLength		= cmdlen;
	entry->scsicmd.scsi_Flags			= SCSIF_READ | SCSIF_AUTOSENSE;

	DoIO((struct IORequest *)entry->req);

	return (entry->scsicmd.scsi_Status ? -1 : 0);
}


static int CDLength(SDL_CD *cdrom)
{
	static const UBYTE	Cmd[10]	=	{ 0x25, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	LONG	statcode;
	statcode=SendCMD(cdrom, Cmd, sizeof(Cmd));

	if (statcode == 0)
	{
		struct MyCDROM	*entry	= CDList[cdrom->id];
		UBYTE	*buf;
		int	length, size;
		UBYTE	byte1,byte2,byte3,byte4;

		buf	= (UBYTE *)entry->inbuf;

		byte1 = buf[0]; byte2 = buf[1]; byte3 = buf[2]; byte4 = buf[3];
		length = (byte1<<24) + (byte2<<16) + (byte3<<8) + byte4;

		byte1 = buf[4]; byte2 = buf[5]; byte3 = buf[6]; byte4 = buf[7];
		size =  (byte1<<24) + (byte2<<16) + (byte3 <<8) + byte4;

		return(length);

	} else {
		return(-1);
	}
}


static const char *SDL_SYS_CDName(int drive)
{
	return(CDList[drive]->dosname);
}

static void SDL_DisposeCD(struct MyCDROM *entry)
{
	DeleteIORequest((struct IORequest *)entry->req);

	DeleteMsgPort((struct MsgPort *)entry->port);

	if (entry->inbuf)
		FreeVecPPC(entry->inbuf);

	if (entry->sensebuf)
		FreeVecPPC(entry->sensebuf);

	entry->req			= NULL;
	entry->inbuf		= NULL;
	entry->sensebuf	= NULL;
}

static int SDL_SYS_CDOpen(int drive)
{
	struct MyCDROM	*entry;
	struct MsgPort	*port;

	/* Yay! Use DOS message port :-) */

	entry	= CDList[drive];
	port	= CreateMsgPort();
	if(port==NULL) {
		return -1;
	} 

	entry->port = port;

	if ((entry->req = CreateIORequest(port, sizeof(struct IOStdReq))) != NULL)
	{
		if ((entry->inbuf		= AllocVecPPC(SCSI_INBUF_SIZE, 0L, 0L)) != NULL)
		{
			if ((entry->sensebuf	= AllocVecPPC(SCSI_SENSE_SIZE, 0L, 0L)) != NULL)
			{
				if (OpenDevice(entry->devname, entry->unit, (struct IORequest *)entry->req, entry->unitflags) == 0)
					{
						entry->req->io_Command	= HD_SCSICMD;
						entry->req->io_Data		= (APTR)&entry->scsicmd;
						entry->req->io_Length	= sizeof(struct SCSICmd);

						return drive;
					}
			}
		}
		SDL_DisposeCD(entry);
	}

	return -1;
}

static void SDL_SYS_CDClose(SDL_CD *cdrom)
{
	struct MyCDROM	*entry	= CDList[cdrom->id];
	if (entry && entry->req)
	{
		CloseDevice((struct IORequest *)entry->req);
		SDL_DisposeCD(entry);
	}
}

static int SDL_SYS_CDGetTOC(SDL_CD *cdrom)
{
	static const UBYTE	Cmd[10] =
		{ 0x43, 0, 0, 0, 0, 0, 0, TOC_SIZE >> 8, TOC_SIZE & 0xff, 0 };
	int	okay;
	okay	= 0;

	if (SendCMD(cdrom, Cmd, sizeof(Cmd)) == 0)
	{
		struct MyCDROM	*entry	= CDList[cdrom->id];
		struct CD_TOC	*toc;
		int	i,CDlen;

		toc	= (struct CD_TOC *)entry->inbuf;
	
		cdrom->numtracks = toc->LastTrack - toc->FirstTrack +1;
		
		if ( cdrom->numtracks > SDL_MAX_TRACKS ) {
			cdrom->numtracks = SDL_MAX_TRACKS;
		}

		/* Read all the track TOC entries */

		okay	= 1;

		for ( i=0; i<cdrom->numtracks; ++i )
		{
			cdrom->track[i].id = i+1;

			if (toc->TOCData[i].Flags & 4)
			{
				cdrom->track[i].type	= SDL_DATA_TRACK;
			} else {
				cdrom->track[i].type	= SDL_AUDIO_TRACK;
			}

			cdrom->track[i].offset		= toc->TOCData[i].Address;
		
			if (i == cdrom->numtracks-1)
			{
				CDlen = CDLength(cdrom);
				if(CDlen != -1) {
					cdrom->track[i].length	= CDlen - cdrom->track[i].offset;
				} else {
					cdrom->track[i].length	=0;
				}
			}
			else
			{
				cdrom->track[i].length		= toc->TOCData[i+1].Address - cdrom->track[i].offset;
			}
		}
	}

	return(okay ? 0 : -1);
}


static CDstatus SDL_SYS_CDStatus(SDL_CD *cdrom, int *position)
{
	static const UBYTE	Cmd[10]	=
		{ 0x42, 0, 0x40, 0x01, 0, 0, 0, SCSI_INBUF_SIZE >> 8, SCSI_INBUF_SIZE & 0xff, 0 };
	CDstatus	status;
	LONG	statcode;
	statcode=SendCMD(cdrom, Cmd, sizeof(Cmd));

	status	= CD_ERROR;
	if (statcode == 0)
	{
		struct MyCDROM	*entry	= CDList[cdrom->id];
		UBYTE	*buf;
		int pos;
		UBYTE	byte1,byte2,byte3,byte4;

		buf	= (UBYTE *)entry->inbuf;

		switch (buf[1])
		{
			case 0x11: status	= CD_PLAYING; break;
			case 0x12: status	= CD_PAUSED; break;
			case 0x13: status	= CD_STOPPED; break;	// Finished
			case 0x15: status	= CD_STOPPED; break;

			default	:
			case 0x14: status	= CD_ERROR; break;
		}

		byte1 = buf[8]; byte2 = buf[9]; byte3 = buf[10]; byte4 = buf[11];
		pos = (byte1<<24) + (byte2<<16) + (byte3<<8) + byte4;

		if (position!=NULL) {
		*position = pos;
		}
	}

	return status;
}

static int SDL_SYS_CDPlay(SDL_CD *cdrom, int start, int length)
{
	UBYTE	Cmd[12]	= { 0xa5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	UWORD	*tmp		= (UWORD *)Cmd;

	tmp[1]	= start >> 16;
	tmp[2]	= start;
	tmp[3]	= length >> 16;
	tmp[4]	= length; 

	SendCMD(cdrom, Cmd, sizeof(Cmd));
	return 0;
}

static int SDL_SYS_CDPause(SDL_CD *cdrom)
{
	static const UBYTE	Cmd[10] =
		{ 0x4b, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	SendCMD(cdrom, Cmd, sizeof(Cmd));
	return 0;
}

static int SDL_SYS_CDResume(SDL_CD *cdrom)
{
	static const UBYTE Cmd[10] = { 0x4b, 0, 0, 0, 0, 0, 0, 0, 1, 0 };
	SendCMD(cdrom, Cmd, sizeof(Cmd));
	return 0;
}

static int SDL_SYS_CDStop(SDL_CD *cdrom)
{
	static const UBYTE Cmd[10] = { 0x4e, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	SendCMD(cdrom, Cmd, sizeof(Cmd));
	return 0;
}

static int SDL_SYS_CDEject(SDL_CD *cdrom)
{
	static const UBYTE	Cmd[6] =
		{ 0x1b, 0, 0, 0, 0x02, 0 };
	SendCMD(cdrom, (UBYTE *)Cmd, sizeof(Cmd));
	return 0;
}

static int SDL_SYS_CDGetVolume(SDL_CD *cdrom, int *vol0, int *vol1)
{
	int statcode;
	static const UBYTE Cmd[10] = { 0x5a, 0x08, 0x0e, 0, 0, 0, 0, SCSI_INBUF_SIZE >> 8, SCSI_INBUF_SIZE & 0xff, 0 };
	statcode = SendCMD(cdrom, Cmd, sizeof(Cmd));

	if (statcode == 0)
	{
		struct MyCDROM	*entry	= CDList[cdrom->id];
		UBYTE	*buf;
		buf	= (UBYTE *)entry->inbuf;

		if (((buf[0]<<8)+buf[1])!= 22 || ((buf[6]<<8)+buf[7])!=0)
		{
			return(-1);
		}

		*vol0 = buf[8+9];
		*vol1 = buf[8+11];
	}

	return statcode;

}

static int SDL_SYS_CDSetVolume(SDL_CD *cdrom, int vol0, int vol1)
{
	int statcode;
	static const UBYTE Cmd[10] = { 0x5a, 0x08, 0x0e, 0, 0, 0, 0, SCSI_INBUF_SIZE >> 8, SCSI_INBUF_SIZE & 0xff, 0 };
	statcode = SendCMD(cdrom, Cmd, sizeof(Cmd));

	if (statcode == 0)
	{
		struct MyCDROM	*entry	= CDList[cdrom->id];
		UBYTE	*buf;
		buf	= (UBYTE *)entry->inbuf;

		if (((buf[0]<<8)+buf[1])!= 22 || ((buf[6]<<8)+buf[7])!=0)
		{
			return(-1);
		}
		else
		{
			static UBYTE Cmd[10] = { 0x55, 0x10, 0, 0, 0, 0, 0, 0, 8+16,  0 };
		
			buf[8+9] = vol0;
			buf[8+11] = vol1;
			statcode = SendCMD(cdrom, Cmd, sizeof(Cmd));
			return(statcode);
		}		
	}

	return(statcode);
}

int  SDL_SYS_CDInit(void)
{
	struct DosList	*dlist;
	ULONG	devices, retval = 1;
	struct MyCDROM **cdlist;

	cdlist	= CDList;
	devices	= 0;
	SDL_numcds = 0;

	SDL_CDcaps.Name = SDL_SYS_CDName;
	SDL_CDcaps.Open = SDL_SYS_CDOpen;
	SDL_CDcaps.GetTOC = SDL_SYS_CDGetTOC;
	SDL_CDcaps.Status = SDL_SYS_CDStatus;
	SDL_CDcaps.Play = SDL_SYS_CDPlay;
	SDL_CDcaps.Pause = SDL_SYS_CDPause;
	SDL_CDcaps.Resume = SDL_SYS_CDResume;
	SDL_CDcaps.Stop = SDL_SYS_CDStop;
	SDL_CDcaps.Eject = SDL_SYS_CDEject;
	SDL_CDcaps.Close = SDL_SYS_CDClose;
	SDL_CDcaps.GetVolume = SDL_SYS_CDGetVolume;
	SDL_CDcaps.SetVolume = SDL_SYS_CDSetVolume;

	dlist	= LockDosList(LDF_DEVICES|LDF_READ);

	while ((dlist = NextDosEntry(dlist, LDF_DEVICES|LDF_READ)) != NULL && devices < MAX_DRIVES)
	{
		struct FileSysStartupMsg	*startup;
		CONST_STRPTR	devname;
		ULONG	devnamelen;

		startup = BADDR(dlist->dol_misc.dol_handler.dol_Startup);
		if ((ULONG) startup > 0x400 &&
		    (startup->fssm_Unit & 0xff000000) == 0 &&
		     startup->fssm_Device && startup->fssm_Environ)
		{
			if ((devname = BADDR(startup->fssm_Device)) != NULL)
			{
				if ((devnamelen	= *devname++) != 0)
				{
					if (FindDup(cdlist, devname, startup->fssm_Unit) == 0)
					{
						struct MyCDROM	*entry;

						if ((entry =  AllocVecPPC(sizeof(*entry), 0L, 0L)) != NULL)
						{
							CONST_STRPTR	dosname;
							ULONG	dosnamelen;

							dosname		= (CONST_STRPTR)BADDR(dlist->dol_Name);
							dosnamelen	= *dosname; dosname++;

							entry->unit			= startup->fssm_Unit;
							entry->unitflags	= startup->fssm_Flags;

							memcpy(entry->dosname, dosname, dosnamelen);
							memcpy(entry->devname, devname, devnamelen);

							entry->dosname[ dosnamelen ]	= '\0';
							entry->devname[ devnamelen ]	= '\0';

							entry->req			= NULL;
							entry->inbuf		= NULL;
							entry->sensebuf	= NULL;

							cdlist[devices] = (struct MyCDROM *)entry;
							devices++;

						}
					}
				}
			}
		}
	}

	UnLockDosList(LDF_DEVICES|LDF_READ);

	if (devices)
	{
		struct IOStdReq	*req;
		struct SCSICmd		*scsicmd;
		struct MsgPort	*port;
		ULONG	i;
		UBYTE	*inbuf, *sensebuf;

		port	= &((struct Process *)FindTask(NULL))->pr_MsgPort;

		if ((req = CreateIORequest(port, sizeof(struct IOStdReq))) != NULL)
		{
			if ((inbuf = AllocVecPPC(SCSI_INBUF_SIZE, 0L, 0L)) != NULL)
			{
				if ((sensebuf = AllocVecPPC(SCSI_SENSE_SIZE, 0L, 0L)) != NULL)
				{
					if ((scsicmd = (struct SCSICmd *)AllocVecPPC(sizeof(*scsicmd), 0L, 0L)) != NULL)
					{
						req->io_Command	= HD_SCSICMD;
						req->io_Data		= (APTR)scsicmd;
						req->io_Length	= sizeof(struct SCSICmd);

						for (i = 0; i < MAX_DRIVES; i++)
						{
							struct MyCDROM	*entry;
							ULONG	is_cdrom;

							entry	= (struct MyCDROM *)cdlist[i];

							if (entry == NULL)
								break;

							is_cdrom	= 0;

							if (OpenDevice(entry->devname, entry->unit, (struct IORequest *)req, entry->unitflags) == 0)
							{
								static const UBYTE	Cmd[]	= { 0x12, 0, 0, 0, 252, 0 };

								scsicmd->scsi_Data			= (UWORD *)inbuf;
								scsicmd->scsi_Length			= SCSI_INBUF_SIZE;
								scsicmd->scsi_SenseActual	= 0;
								scsicmd->scsi_SenseData		= sensebuf;
								scsicmd->scsi_SenseLength	= SCSI_SENSE_SIZE;
								scsicmd->scsi_CmdActual		= 0;
								scsicmd->scsi_Command		= (UBYTE *)Cmd;
								scsicmd->scsi_CmdLength		= sizeof(Cmd);
								scsicmd->scsi_Flags			= SCSIF_READ | SCSIF_AUTOSENSE;

								DoIO((struct IORequest *)req);

								if (scsicmd->scsi_Status == 0 && req->io_Error == 0)
								{
									/* 5 is for CDROM (maybe not proper checking) */
									/* Must be removable as well*/

									if ((inbuf[0] & 5) == 5 && inbuf[1] & 0x80)
										is_cdrom	= 1;
								}

								CloseDevice((struct IORequest *)req);
							}

							if (is_cdrom == 1)
							{
								cdlist[SDL_numcds]	= cdlist[i];
								SDL_numcds++;
							}
						}

						FreeVecPPC(scsicmd);
					}

					FreeVecPPC(sensebuf);
				}

				FreeVecPPC(inbuf);
			}

			DeleteIORequest((struct IORequest *)req);
		}
	}

	return(0);
}

void SDL_SYS_CDQuit(void)
{
}
