################################################################################
#
################################################################################

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
LOCAL_SRC_FILES += AcdkCCAPTest.cpp

#-----------------------------------------------------------
LOCAL_C_INCLUDES += \
    $(MTK_PATH_SOURCE)/hardware/meta/common/inc \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/include \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/acdk/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/acdk/inc/cct \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/acdk/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/acdk/inc/acdk \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/acdk/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/acdk/inc \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/acdk/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/inc/cct \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/acdk/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/include \
    $(MTK_PATH_SOURCE)/external/mhal/src/custom/inc \
    $(MTK_PATH_SOURCE)/external/mhal/inc \
    $(MTK_PATH_COMMON)/kernel/imgsensor/inc \
    $(MTK_PATH_CUSTOM)/hal/inc \
    $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc \
    $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa \

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)
#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES +=
#
LOCAL_STATIC_LIBRARIES += libacdk_entry_cctif
LOCAL_STATIC_LIBRARIES += libacdk_entry_mdk

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog libcutils
LOCAL_SHARED_LIBRARIES += libdl

#-----------------------------------------------------------

LOCAL_MODULE := ccaptest
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#-----------------------------------------------------------
ifneq (yes,$(strip $(MTK_EMULATOR_SUPPORT)))
include $(MTK_EXECUTABLE)
endif
