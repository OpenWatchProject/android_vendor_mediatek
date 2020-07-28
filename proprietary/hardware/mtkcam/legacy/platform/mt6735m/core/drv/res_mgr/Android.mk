#
# libcamdrv_res_mgr
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#
LOCAL_SRC_FILES := \
    res_mgr_drv.cpp

#
LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc/common \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/inc/drv \
    $(TOP)/$(MTK_MTKCAM_PLATFORM)/core/src/drv/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
    $(TOP)/$(MTK_ROOT)/hardware/bwc/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/kernel/drivers/hdmitx \
    $(TOP)/bionic/libc/kernel/common/linux/mt6582 \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include/mtkcam \

# Add a define value that can be used in the code to indicate that it's using LDVT now.
# For print message function when using LDVT.
# Note: It will be cleared by "CLEAR_VARS", so if it is needed in other module, it
# has to be set in other module again.
ifeq ($(BUILD_MTK_LDVT),true)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
endif

#

#
LOCAL_MODULE := libcamdrv_res_mgr
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#
#LOCAL_MULTILIB := 32
#
# Start of common part ------------------------------------
sinclude $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
$(info $$CUSTOM_HAL_IMGSENSOR is [${CUSTOM_HAL_IMGSENSOR}])

#for Android O use fake 3m/5m/8m D2 project use only
MTKCAM_FAKE_SENSOR := 0
ifneq (,$(findstring _5mp,$(CUSTOM_HAL_IMGSENSOR)))
    $(info $$use fake 5m sensor)
    MTKCAM_FAKE_SENSOR := 1
else
    ifneq (,$(findstring _8mp,$(CUSTOM_HAL_IMGSENSOR)))
        $(info $$use fake 8m sensor)
        MTKCAM_FAKE_SENSOR := 1
    else
        ifneq (,$(findstring _3mp,$(CUSTOM_HAL_IMGSENSOR)))
            $(info $$use fake 3m sensor)
            MTKCAM_FAKE_SENSOR := 1
        endif
    endif
endif

LOCAL_CFLAGS += -DMTKCAM_FAKE_SENSOR=$(MTKCAM_FAKE_SENSOR)
#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include


#-----------------------------------------------------------
LOCAL_C_INCLUDES +=$(TOP)/$(MTK_MTKCAM_PLATFORM)/include
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES +=$(TOP)/$(MTKCAM_C_INCLUDES)/..

LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/
LOCAL_HEADER_LIBRARIES += libmtkcam_headers
# End of common part ---------------------------------------
#
include $(MTK_STATIC_LIBRARY)



#
#include $(call all-makefiles-under, $(LOCAL_PATH))
