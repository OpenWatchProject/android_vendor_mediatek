LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES:= Meta_CCAP_Para.cpp \
                  meta_ccap_interface.cpp

LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/include \
    $(MTK_PATH_SOURCE)/hardware/meta/common/inc \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/acdk/inc/cct \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/acdk/inc/acdk \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/acdk/inc \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/include \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include/mtkcam \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include/ \
    $(MTK_PATH_SOURCE)/external/mhal/src/custom/inc \
    $(MTK_PATH_SOURCE)/external/mhal/inc \
    $(MTK_PATH_COMMON)/kernel/imgsensor/inc \
    $(MTK_PATH_CUSTOM)/hal/inc \
    $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc \
    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/cgen/cfgfileinc \
    $(TOP)/$(MTK_PATH_CUSTOM)/cgen/cfgfileinc

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES += libacdk_entry_cctif
LOCAL_WHOLE_STATIC_LIBRARIES += libacdk_entry_mdk

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES := libcutils libc libc++
LOCAL_SHARED_LIBRARIES += liblog

LOCAL_MODULE := libccap
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include


include $(MTK_SHARED_LIBRARY)

