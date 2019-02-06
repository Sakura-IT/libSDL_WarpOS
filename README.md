libSDL_wos
==========

This is a port of the SDL 1.2.14 library to WarpOS. It is to be used with
the gcc-mos2wos package on Aminet. Use either the standard or altivec
makefile provided.

Bolted on top is MiniGL (for orignal sources see Aminet) with the addition
of a few extra functions to facilitate SDL to provide the screen/window
instead of MiniGL. The GL part can be compiled with the gl makefile.

The nix make file will compile using the libnix C library, the other makefiles
use newlib as the C library.

Thanks to Arti for providing the initial sources as part of the Apollo SDL
project.
