NOTICE:
SRC (sampling rate converter) is needed whenever two systems working at different sampling rate are to be interconnected.
In this audio SRC system, a band-limited interpolation method is used. We call our sampling rate converter system BLI SRC.
In BLI SRC, we provide conversions between the 9 standard rates to 44.1K and 48K.
( 9 standard rates : 48 kHz, 44.1 kHz, 32 kHz, 24 kHz, 22.05 kHz, 16 kHz, 12 kHz, 11.025 kHz, 8 kHz)

The library, libblisrc.a, is built and can be linked already.
For people who may like to use these APIs, just includes blisrc_exp.h, and see the definitions in header file.
