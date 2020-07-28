#
# Copyright (c) 2016 MediaTek Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files
# (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software,
# and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

#ifeq ($(MTK_SOTER_SUPPORT), yes)
#
#LOCAL_PATH    := $(call my-dir)
#
#include $(CLEAR_VARS)
#
#LOCAL_MODULE := libkeystore_soterutils
#LOCAL_PROPRIETARY_MODULE := true
#LOCAL_MODULE_OWNER := mtk
#LOCAL_MODULE_TAGS := debug eng optional
#
## Add new source files here
#LOCAL_SRC_FILES +=\
#    keystore_soterutils.cpp
#
#LOCAL_C_INCLUDES +=\
#    external/boringssl/include \
#    system/core/include \
#
#LOCAL_C_INCLUDES += vendor/mediatek/proprietary/hardware/keymaster/include
#LOCAL_C_INCLUDES += system/security/keystore/include
#LOCAL_C_INCLUDES += system/security/keystore/
#
#LOCAL_CFLAGS += -DSOTER
#LOCAL_FLANG_CFLGAS += -DSOTER
#
##LOCAL_SHARED_LIBRARIES := libMcClient liblog libMcTeeKeymaster libcrypto
#
#include $(MTK_SHARED_LIBRARY)
#
#endif # TRUSTONIC_KEYMASTER_LEGACY_SUPPORT
