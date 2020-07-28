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
LOCAL_MODULE:= libfodca_core_headers
LOCAL_EXPORT_C_INCLUDE_DIRS += \
    $(LOCAL_PATH) \
    $(LOCAL_PATH)/../drv_api/public \
    $(LOCAL_PATH)/../include \
    vendor/mediatek/proprietary/trustzone/common/hal/source/trustlets/secmem/common/drv_api_inc
include $(BUILD_HEADER_LIBRARY)

###############################################################################
##### protected memory GP TA
###############################################################################
include $(CLEAR_VARS)

# Feature Option
PMEM_TA_DEBUG_ENABLE := no

# OUTPUT_NAME
LOCAL_MODULE := tafod_core
TA_PROP_GROUP_ID := dev_ta

# TBASE_API_LEVEL
LOCAL_CFLAGS += -DTBASE_API_LEVEL=5
LOCAL_CFLAGS += -Werror -Wall -Wextra
LOCAL_CFLAGS += -fno-short-enums

ifeq ($(PMEM_TA_DEBUG_ENABLE),yes)
LOCAL_CFLAGS += -DPMEM_TA_DEBUG_ENABLE
endif

### Add include path here
LOCAL_HEADER_LIBRARIES += libfodca_core_headers

### Add source code files for C++ compiler here
LOCAL_SRC_FILES += fod_gp.c

include vendor/mediatek/proprietary/trustzone/custom/build/tee_static_library.mk

endif # End of MTK_TEE_GP_SUPPORT=yes
