LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES:= Meta_CCAP_Para.cpp \
                  meta_ccap_interface.cpp
$(info MTK_PATH_SOURCE = $(MTK_PATH_SOURCE))
LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include/mtkcam \
    $(MTK_PATH_SOURCE)/external/meta/common/inc \
    $(MTK_PATH_SOURCE)/hardware/meta/common/inc \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/mt6735/acdk/inc/cct \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/mt6735/acdk/inc/acdk \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/mt6735/acdk/inc \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/acdk/inc/cct \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/acdk/inc/acdk \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/acdk/inc \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/include \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include/mtkcam/ \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include/ \
    $(MTK_PATH_SOURCE)/external/mhal/src/custom/inc \
    $(MTK_PATH_SOURCE)/external/mhal/inc \
    $(MTK_PATH_COMMON)/kernel/imgsensor/inc \
    $(MTK_PATH_CUSTOM)/hal/inc \
    $(MTK_PATH_SOURCE)/custom/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/hal/D1/inc \
    $(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/inc \


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

#
# Start of common part ------------------------------------


#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include/D1

# End of common part ---------------------------------------
#
include $(MTK_SHARED_LIBRARY)

