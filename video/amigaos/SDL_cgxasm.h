#ifndef _SDL_CGXASM_H
#define _SDL_CGXASM_H

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id$";
#endif

#pragma pack(2)
#include <exec/types.h>
#pragma pack()

//extern void (*copy_and_swap16)(APTR src, APTR dst, LONG pixels);
//extern void (*copy_and_swap32)(APTR src, APTR dst, LONG pixels);

void copy_and_swap16_generic(APTR src, APTR dst, LONG pixels);
void copy_and_swap32_generic(APTR src, APTR dst, LONG pixels);
void copy_and_swap16(APTR src, APTR dst, LONG pixels);
void copy_and_swap32(APTR src, APTR dst, LONG pixels);
void copy_and_swap16_altivec(APTR src, APTR dst, LONG pixels);
void copy_and_swap32_altivec(APTR src, APTR dst, LONG pixels);

#endif /* _SDL_CGXASM_H */
