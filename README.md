libSDL_wos
==========

This is a port of the SDL 1.2.15 library to WarpOS.

Bolted on top is MiniGL (for orignal sources see Aminet) with the addition
of a few extra functions to facilitate SDL to provide the screen/window
instead of MiniGL. The GL part can be compiled with the gl makefile.

The extra functions are: mglAttachContext, mglDetachContext, MGLMiniTerm.

# Requirements

This library can be compiled using SDK 3.12 from MorphOS in combination
with NDK3.9, gcc-mos2wos-lite (Aminet) and the latest vbcc for MorphOS with
target aos68k.

# Thanks

To Arti for providing the initial sources as part of the Apollo SDL project.

To Henryk Richter for the threads update.
