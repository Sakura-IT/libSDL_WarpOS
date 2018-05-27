/*
 * Copyright (C) 2007 MorphOS Development Team
 *
 * $Id$
 */


#pragma pack(2)
#include <exec/types.h>
#pragma pack()

#include "SDL_types.h"
#include "SDL_cpuinfo.h"
#include "SDL_cgxasm.h"

#if SDL_ALTIVEC_COPY
#include <hardware/vector.h>
#ifdef HAVE_ALTIVEC_H
#include <altivec.h>
#endif
#endif

void copy_and_swap16_generic(APTR src, APTR dst, LONG size);
asm("copy_and_swap16_generic:\n\
	li     0,0\n\
	andi.  9,4,3\n\
	dcbt   0,3\n\
	li     11,32\n\
	addi   4,4,-2\n\
	beq    .was_even\n\
	lhbrx  7,3,0\n\
	addi   5,5,-1\n\
	addi   3,3,2\n\
	sthu   7,2(4)\n\
.was_even:\n\
	cmpwi  5,7\n\
	li     10,4\n\
	ble   .tailcopy\n\
	addi   4,4,-2\n\
.loop:\n\
	lwbrx  6,3,0\n\
	addi   5,5,-8\n\
	dcbt   11,3\n\
	lwbrx  7,3,10\n\
	cmpwi  5,7\n\
	rlwimi 6,6,16,0,31\n\
	addi   3,3,8\n\
	rlwimi 7,7,16,0,31\n\
	lwbrx  8,3,0\n\
	stw    6,4(4)\n\
	lwbrx  9,3,10\n\
	rlwimi 8,8,16,0,31\n\
	stw    7,8(4)\n\
	stw    8,12(4)\n\
	rlwimi 9,9,16,0,31\n\
	addi   3,3,8\n\
	stwu   9,16(4)\n\
	bgt    .loop\n\
	addi   4,4,2\n\
.tailcopy:\n\
	cmpwi  5,0\n\
	subf   0,4,3\n\
	mtctr  5\n\
	bclr   4,1\n\
.tail:\n\
	lhbrx  6,4,0\n\
	sthu   6,2(4)\n\
	bdnz   .tail\n\
	blr");

void copy_and_swap32_generic(APTR src, APTR dst, LONG size);
asm("copy_and_swap32_generic:\n\
	li     0,0\n\
	andi.  9,4,3\n\
	dcbt   0,3\n\
	li     11,32\n\
	addi   4,4,-4\n\
	beq    .was_even4\n\
	lwbrx  7,3,0\n\
	addi   5,5,-1\n\
	addi   3,3,4\n\
	stwu   7,4(4)\n\
.was_even4:\n\
	cmpwi  5,3\n\
	li     10,4\n\
	ble   .tailcopy4\n\
	addi   4,4,-4\n\
.loop4:\n\
	lwbrx  6,3,0\n\
	addi   5,5,-4\n\
	dcbt   11,3\n\
	lwbrx  7,3,10\n\
	addi   3,3,8\n\
	cmpwi  5,7\n\
	lwbrx  8,3,0\n\
	stw    6,4(4)\n\
	lwbrx  9,3,10\n\
	stw    7,8(4)\n\
	stw    8,12(4)\n\
	addi   3,3,8\n\
	stwu   9,16(4)\n\
	bgt    .loop4\n\
	addi   4,4,4\n\
.tailcopy4:\n\
	cmpwi  5,0\n\
	subf   0,4,3\n\
	mtctr  5\n\
	bclr   4,1\n\
.tail4:\n\
	lwbrx  6,4,0\n\
	stwu   6,4(4)\n\
	bdnz   .tail4\n\
	blr");

void copy_and_swap16(APTR srcx, APTR destx, LONG pixels)
{
	#if SDL_ALTIVEC_COPY
	if (SDL_HasAltiVec())
		{
		copy_and_swap16_altivec(srcx, destx, pixels);
		}
	else
	#endif
		{
		copy_and_swap16_generic(srcx, destx, pixels);
		}
}

void copy_and_swap32(APTR srcx, APTR destx, LONG pixels)
{
	#if SDL_ALTIVEC_COPY
	if (SDL_HasAltiVec())
		{
		copy_and_swap32_altivec(srcx, destx, pixels);
		}
	else
	#endif
		{
		copy_and_swap32_generic(srcx, destx, pixels);
		}
}

#if SDL_ALTIVEC_COPY

/* AltiVec routines written by Grzegorz Kraszewski
 *
 * Length is given in pixels (number of M68k UWORDs to swap)
 */

void copy_and_swap16_altivec(APTR srcx, APTR destx, LONG pixels)
{
	if (pixels >= 32 && SDL_HasAltiVec())
	{
		ULONG p1, p2;

		/* Test if the same alignment and not an odd address */

		p1 = (ULONG)srcx & 0xf;
		p2 = (ULONG)destx & 0xf;

		if (p1 == p2 && (!p1 || ((!(p1 & 1)) && pixels >= 40)))
		{
			VECTOR_UBYTE pr = (VECTOR_UBYTE)vec_rl((VECTOR_UWORD)vec_lvsl(0, (short*)NULL), vec_splat_u16(8));	//(VECTOR_UBYTE)VEC_VALUE(0x01, 0x00, 0x03, 0x02, 0x05, 0x04, 0x07, 0x06, 0x09, 0x08, 0x0B, 0x0A, 0x0D, 0x0C, 0x0F, 0x0E);
			VECTOR_UBYTE lda, ldb, ldc, ldd;
			VECTOR_UBYTE *src;
			VECTOR_UBYTE *dest;

			if (p1)
			{
				/* Was not aligned to 16 bytes, copy few bytes first... */

				ULONG pix = (16 - p1) / 2;

				copy_and_swap16_generic(srcx, destx, pix);

				pixels -= pix;
				srcx    = (APTR)((ULONG)srcx + pix * 2);
				destx   = (APTR)((ULONG)destx + pix * 2);
			}

			src = (VECTOR_UBYTE*)srcx;
			dest = (VECTOR_UBYTE*)destx;

			do
			{
				vec_dstt(src + 8, VEC_DST_BLOCKS(320, 1, 320), 0);
				lda = *src++;
				ldb = *src++;
				pixels -= 32;
				ldc = *src++;
				*dest++ = vec_perm(lda, lda, pr);
				ldd = *src++;
				*dest++ = vec_perm(ldb, ldb, pr);
				*dest++ = vec_perm(ldc, ldc, pr);
				*dest++ = vec_perm(ldd, ldd, pr);
			} while (pixels >= 32);

			srcx = src;
			destx = dest;
		}
	}

	if (pixels > 0)
	{
		copy_and_swap16_generic(srcx, destx, pixels);
	}
}

void copy_and_swap32_altivec(APTR srcx, APTR destx, LONG pixels)
{
	if (pixels >= 32 && SDL_HasAltiVec())
	{
		ULONG p1, p2;

		/* Test if the same alignment and not an odd address */

		p1 = (ULONG)srcx & 0xf;
		p2 = (ULONG)destx & 0xf;

		if (p1 == p2 && (!p1 || ((!(p1 & 3)) && pixels >= 40)))
		{
			VECTOR_UBYTE pr = vec_xor(vec_lvsl(0, (short*)NULL), vec_splat_u8(3));
			VECTOR_UBYTE lda, ldb, ldc, ldd;
			VECTOR_UBYTE *src;
			VECTOR_UBYTE *dest;

			if (p1)
			{
				/* Was not aligned to 16 bytes, copy few bytes first... */

				ULONG pix = (16 - p1) / 4;

				copy_and_swap32_generic(srcx, destx, pix);

				pixels -= pix;
				srcx    = (APTR)((ULONG)srcx + pix * 4);
				destx   = (APTR)((ULONG)destx + pix * 4);
			}

			src = (VECTOR_UBYTE*)srcx;
			dest = (VECTOR_UBYTE*)destx;

			do
			{
				vec_dstt(src + 8, VEC_DST_BLOCKS(320, 1, 320), 0);
				lda = *src++;
				ldb = *src++;
				pixels -= 16;
				ldc = *src++;
				*dest++ = vec_perm(lda, lda, pr);
				ldd = *src++;
				*dest++ = vec_perm(ldb, ldb, pr);
				*dest++ = vec_perm(ldc, ldc, pr);
				*dest++ = vec_perm(ldd, ldd, pr);
			} while (pixels >= 16);

			srcx = src;
			destx = dest;
		}
	}

	if (pixels > 0)
	{
		copy_and_swap32_generic(srcx, destx, pixels);
	}
}
#endif