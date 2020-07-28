LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

#-----------------------------------------------------------

LOCAL_ARM_MODE := arm
LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES:= Meta_CCAP_Para.cpp \
                  meta_ccap_interface.cpp

LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/meta/common/inc \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/mt6735m/inc/acdk \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/mt6735m/acdk/inc/cct \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/mt6735m/acdk/inc/acdk \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/mt6735m/include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include/mtkcam \
    $(MTK_PATH_SOURCE)/external/mhal/src/custom/inc \
    $(MTK_PATH_SOURCE)/external/mhal/inc \
    $(MTK_PATH_COMMON)/kernel/imgsensor/inc \
    $(MTK_PATH_CUSTOM)/hal/inc \
    $(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc \
    $(TOP)/$(MTK_PATH_CUSTOM)/cgen/cfgfileinc \
    $(TOP)/device/mediatek/sprout/kernel-headers \

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES += libacdk_entry_cctif
LOCAL_WHOLE_STATIC_LIBRARIES += libacdk_entry_mdk_cct

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libccci_util

LOCAL_MODULE := libccap
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include/D2

include $(MTK_SHARED_LIBRARY)

