This directory contains files for the MTK extended Neural Networks API.

CONTENTS OF THIS DIRECTORY

./runtime: Implementation of the MTK extended NN API runtime.
           Includes source code and internal header files.

./runtime/include: The header files that an external developer would use.
                   These will be packaged with the NDK.  Includes a
                   C++ wrapper around the C API to make it easier to use.
./runtime/test: Test files.

./common: Contains files that can be useful for multiple components,
          e.g. runtime.  Includes source code and internal header files.
./common/include: Header files to be used by the components using common.


WHAT IT DOES?
=============
It contains the Mediatek framework's customization for neual networks, named neuropilot.

HOW IT WAS BUILT?
==================
Build command:mmm vendor/mediatek/proprietary/frameworks/ml/nn
And it generates libneuropilot.so in system/lib and system/lib64(in 64bit product).

HOW TO USE IT?
==============
Add libneuropilot.so to PRODUCT_PACKAGES to use it


