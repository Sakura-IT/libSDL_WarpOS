libSDL_wos
==========

This is a port of the SDL 1.2.15 library to WarpOS. It is to be used with
the gcc-mos2wos-lite package on Aminet. Use either the standard or altivec
makefile provided.

Bolted on top is MiniGL (for orignal sources see Aminet) with the addition
of a few extra functions to facilitate SDL to provide the screen/window
instead of MiniGL. The GL part can be compiled with the gl makefile.

The extra functions are: mglAttachContext, mglDetachContext, MGLMiniTerm.

Thanks to Arti for providing the initial sources as part of the Apollo SDL
project.

Thanks to Henryk Richter for the threads update.
