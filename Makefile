# Makefile for gcc version of SDL
		  
CC := gcc 
AS := as
LD := ld
RL := ranlib

VC  := vc
VLD := vlink
VCDEFINES = -DWARPOS -D__SIZE_T
VCFLAGS = +aos68k -c99 -O3 -Iinclude -Igg:ppc-warpos/os-includeppc -Igg:ppc-warpos/include 

CFLAGS = --specs=warpup -maltivec -I. -I../include -I./include/ -I./include/SDL -O2 -fomit-frame-pointer
GCCDEFINES = -DNO_AMIGADEBUG -DWARPOS -DSDL_ALTIVEC_COPY -DSDL_ALTIVEC_BLITTERS

GOBJS = audio/SDL_audio.go audio/SDL_audiocvt.go audio/SDL_mixer.go audio/SDL_wave.go audio/amigaos/SDL_ahiaudio.go \
	main/SDL_error.go main/SDL_fatal.go video/SDL_RLEaccel.go video/SDL_blit.go video/SDL_blit_0.go \
	video/SDL_blit_1.go video/SDL_blit_A.go video/SDL_blit_N.go \
	video/SDL_bmp.go video/SDL_cursor.go video/SDL_pixels.go video/SDL_surface.go video/SDL_stretch.go \
	video/SDL_yuv.go video/SDL_yuv_sw.go video/SDL_video.go cpuinfo/SDL_cpuinfo.go \
	timer/amigaos/SDL_systimer.go timer/SDL_timer.go joystick/SDL_joystick.go video/amigaos/SDL_cgxasm.go \
	joystick/amiga/SDL_sysjoystick.go cdrom/SDL_cdrom.go cdrom/amiga/SDL_syscdrom.go events/SDL_quit.go events/SDL_active.go \
	events/SDL_keyboard.go events/SDL_mouse.go events/SDL_resize.go file/SDL_rwops.go main/SDL.go \
	events/SDL_events.go thread/amigaos/SDL_sysmutex.go thread/amigaos/SDL_syssem.go thread/amigaos/SDL_systhread.go thread/amigaos/SDL_thread.go \
	thread/amigaos/SDL_syscond.go video/amigaos/SDL_cgxvideo.go video/amigaos/SDL_cgxmodes.go video/amigaos/SDL_cgximage.go video/amigaos/SDL_amigaevents.go \
	video/amigaos/SDL_amigamouse.go video/amigaos/SDL_cgxgl.go video/amigaos/SDL_cgxwm.go \
	video/amigaos/SDL_cgxyuv.go video/amigaos/SDL_cgxaccel.go video/amigaos/SDL_cgxgl_wrapper.go \
	video/SDL_gamma.go stdlib/SDL_stdlib.go stdlib/SDL_string.go stdlib/SDL_malloc.go stdlib/SDL_getenv.go

VOBJS = video/amigaos/SDL_cgxwm_68k.o video/amigaos/SDL_amigaevents_68k.o audio/amigaos/SDL_wait_68k.o
VOBJSELF = video/amigaos/SDL_cgxwm_68k.go video/amigaos/SDL_amigaevents_68k.go audio/amigaos/SDL_wait_68k.go

%_68k.o: %_68k.c
	$(VC) $(VCFLAGS) $(VCDEFINES) -c $*_68k.c -o $@
	vlink -belf32morphos -set-deluscore -r $@ -o $*_68k.go

%.go: %.c
	$(CC) $(CFLAGS) $(GCCDEFINES) -o $@ -c $*.c

%.ll: %.s
	$(AS) -o $@ $*.s

all: libSDL.a

libSDL.a: $(GOBJS) $(VOBJS)
	-rm -f libSDL.a
	ar cru libSDL.a $(GOBJS) $(VOBJSELF)
	$(RL) libSDL.a

clean:
	-rm -f $(GOBJS) $(VOBJS) $(VOBJSELF)
