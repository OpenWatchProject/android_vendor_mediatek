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

ifeq ($(strip $(CFG_GZ_SECURE_DSP)), yes)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:= libdrfod_core_headers
LOCAL_EXPORT_C_INCLUDE_DIRS += \
    $(LOCAL_PATH)/ \
    $(LOCAL_PATH)/../drv_api/public \
    $(LOCAL_PATH)/../include \
    vendor/mediatek/proprietary/bootable/bootloader/preloader/platform/$(MTK_PLATFORM_DIR)/src/security/trustzone/inc \
    vendor/mediatek/proprietary/trustzone/common/hal/source/trustlets/secmem/common/drv_api_inc
include $(BUILD_HEADER_LIBRARY)


###############################################################################
##### protected memory core library
###############################################################################
include $(CLEAR_VARS)

# Feature Option
PMEM_TDRV_DEBUG_ENABLE := no

# OUTPUT_NAME
LOCAL_MODULE := drfod_core
TA_PROP_GROUP_ID := dev_drv

DRIVER_VENDOR_ID := 9
DRIVER_NUMBER := 0x1979
DRIVER_ID := $$(($(DRIVER_VENDOR_ID)<<16|$(DRIVER_NUMBER)))


LOCAL_CFLAGS += -DDRIVER_ID=$(DRIVER_ID)
LOCAL_CFLAGS += -DTBASE_API_LEVEL=5
LOCAL_CFLAGS += -Werror -Wall -Wextra
LOCAL_CFLAGS += -fshort-wchar
LOCAL_CFLAGS += -fno-short-enums

ifeq ($(PMEM_TDRV_DEBUG_ENABLE),yes)
LOCAL_CFLAGS += -DPMEM_TDRV_DEBUG_ENABLE
endif

ifneq ($(MTK_PLATFORM_DIR), mt6768)
LOCAL_CFLAGS += -DVPU_M4U_ENABLE
endif

ifeq ($(MTK_PLATFORM_DIR), mt6768)
LOCAL_CFLAGS += -DWITHOUT_DSP
endif

ifeq ($(TRUSTONIC_TEE_SUPPORT),yes)
LOCAL_CFLAGS += -DTRUSTONIC_TEE_ENABLED
endif

LOCAL_SRC_FILES += fod_tdrv.c

### Add include path here
LOCAL_HEADER_LIBRARIES += libdrfod_core_headers

include vendor/mediatek/proprietary/trustzone/custom/build/tee_static_library.mk

endif # MTK_TEE_GP_SUPPORT = yes
