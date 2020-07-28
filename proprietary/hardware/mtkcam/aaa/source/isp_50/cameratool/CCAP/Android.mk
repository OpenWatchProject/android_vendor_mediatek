LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := Meta_CCAP_Para.cpp \
                   ../../../common/cameratool/CCAP/meta_ccap_interface.cpp

$(info MTK_PATH_SOURCE = $(MTK_PATH_SOURCE))
LOCAL_C_INCLUDES += \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/include \
    $(MTK_PATH_SOURCE)/external/mhal/src/custom/inc \
    $(MTK_PATH_SOURCE)/external/mhal/inc \
    $(MTK_PATH_COMMON)/kernel/imgsensor/inc \
    $(MTK_PATH_CUSTOM)/hal/inc \
    $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc \
    $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa \

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)
#For BuildInTypes.h
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include/mtkcam/def
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/meta/common/inc
#LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/ccci/include
#LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/meta/misc/wifion
#LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/meta/misc/font
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/common/cameratool/CCAP
#For AEPlineTable.h
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/hal/inc/custom/aaa
#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES += libacdk_entry_cctif

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES := libcutils libc libc++

LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libccci_util
#For AE
LOCAL_SHARED_LIBRARIES += lib3a.ae.core

LOCAL_MODULE := libccap
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#-----------------------------------------------------------
LOCAL_CFLAGS += -DCAM3_ISP_CCAP_EN=1

#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)

