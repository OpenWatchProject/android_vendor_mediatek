#
# Copyright (c) 2018 MediaTek Inc.
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

ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)

LOCAL_PATH := $(call my-dir)

###############################################################################
##### protected memory API library
###############################################################################
include $(CLEAR_VARS)

# Feature Option
PMEM_DRVAPI_DEBUG_ENABLE := no

# OUTPUT_NAME
LOCAL_MODULE := drpmem_api

DRIVER_VENDOR_ID := 9
DRIVER_NUMBER := 0x3419
DRIVER_ID := $$(($(DRIVER_VENDOR_ID)<<16|$(DRIVER_NUMBER)))

LOCAL_CFLAGS += -DDRIVER_ID=$(DRIVER_ID)

LOCAL_CFLAGS += -DTBASE_API_LEVEL=5
LOCAL_CFLAGS += -Werror -Wall -Wextra
LOCAL_CFLAGS += -fshort-wchar
LOCAL_CFLAGS += -fno-short-enums
LOCAL_CFLAGS += -Wno-unused-function

ifeq ($(PMEM_DRVAPI_DEBUG_ENABLE),yes)
LOCAL_CFLAGS += -DPMEM_DRVAPI_DEBUG_ENABLE
endif

LOCAL_SRC_FILES += drpmem_api.c

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/public \
	$(LOCAL_PATH)/../include

LOCAL_EXPORT_C_INCLUDE_DIRS += $(LOCAL_PATH)/public

include vendor/mediatek/proprietary/trustzone/custom/build/tee_static_library.mk

endif # MTK_TEE_GP_SUPPORT = yes
