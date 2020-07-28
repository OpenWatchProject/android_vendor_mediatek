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

# Only build when GZ is enabled
ifeq ($(CFG_GZ_SECURE_DSP),yes)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:= libfod_ca_headers
LOCAL_EXPORT_C_INCLUDE_DIRS += \
    "device/mediatek/common/kernel-headers" \
    "system/core/include" \
    "vendor/mediatek/proprietary/external/libion_mtk/include" \
    "vendor/mediatek/proprietary/geniezone/external/uree/include"
include $(BUILD_HEADER_LIBRARY)

# ====================================================================================
# pmem_share - basic tests of share memory read/write between GZ and TEE.
#              It will go through libion, kernel ION/pmem/SSMR, GZ and TEE.
# ====================================================================================
include $(CLEAR_VARS)

# Feature option
PMEM_MOCK_GZ_SUPPORT := no
TEE_MEMORY_MAP_TEST_SUPPORT := no
TEE_MEMORY_MAP_NON_SECURE := yes

LOCAL_MODULE := fod_ca
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

### Add include path here
LOCAL_HEADER_LIBRARIES += libfod_ca_headers

#ifneq (,$(filter userdebug eng,$(TARGET_BUILD_VARIANT)))
#LOCAL_CFLAGS += -DPMEM_DEBUG_BUILD
#endif

LOCAL_CFLAGS += -Werror

# Force enable mock if GZ is not enabled
ifeq ($(MTK_ENABLE_GENIEZONE),yes)
ifneq ($(PMEM_MOCK_GZ_SUPPORT),yes)
LOCAL_SRC_FILES += fod_ca.c
LOCAL_STATIC_LIBRARIES += libgz_uree
endif # end of PMEM_MOCK_GZ_SUPPORT
else
PMEM_MOCK_GZ_SUPPORT := yes
endif

ifeq ($(TEE_MEMORY_MAP_TEST_SUPPORT),yes)
LOCAL_CFLAGS += -DTEE_MEMORY_MAP_TEST_ENABLE
endif

ifeq ($(TEE_MEMORY_MAP_NON_SECURE),yes)
LOCAL_CFLAGS += -DTEE_MEMORY_MAP_NS_FLAGS
endif

#LOCAL_SRC_FILES += pmem_share.cpp

#ifeq ($(PMEM_MOCK_GZ_SUPPORT),yes)
#LOCAL_SRC_FILES += pmem_mock.cpp
#LOCAL_CFLAGS += -DPMEM_MOCK_GZ
#endif

#LOCAL_SRC_FILES += pmem_tee.cpp
#LOCAL_SRC_FILES += pmem_lib.cpp

LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libion libion_mtk

ifeq ($(MTK_TEE_GP_SUPPORT),yes)
LOCAL_CFLAGS += -DGP_TEE_ENABLED
endif

ifeq ($(TRUSTONIC_TEE_SUPPORT),yes)
LOCAL_SHARED_LIBRARIES += libMcClient
LOCAL_CFLAGS += -DTRUSTONIC_TEE_ENABLED
LOCAL_CFLAGS += -DTBASE_API_LEVEL=5
endif
ifeq ($(MICROTRUST_TEE_SUPPORT),yes)
LOCAL_SHARED_LIBRARIES += libTEECommon
endif
include $(BUILD_NATIVE_TEST)

endif # end of MTK_ENABLE_GENIEZONE
