#!/bin/bash

CFLAGS='-O0 -gdwarf-2' \
CROSS_COMPILE=../../../prebuilts/gcc/linux-x86/aarch64/linaro-aarch64-linux-gnu-4.8/bin/aarch64-linux-gnu- \

#make DEBUG=1 PLAT=mt6752 all
make DEBUG=1 PLAT=mt6795 all

#make DEBUG=1 PLAT=mt6752 SPD=tspd all


