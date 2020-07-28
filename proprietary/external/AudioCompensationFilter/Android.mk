LOCAL_PATH := $(call my-dir)

############################################################################################################### System
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
AudioCompFltCustParam.cpp

LOCAL_C_INCLUDES := \
    system/core/base/include \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/cgen/cfgfileinc

LOCAL_SHARED_LIBRARIES := \
    libaudioclient \
    libcutils \
    liblog \
    libutils

LOCAL_C_INCLUDES += \
        external/libxml2/include \
        external/icu/libandroidicu/include \
        $(JNI_H_INCLUDE) \
        $(MTK_PATH_SOURCE)/external/AudioParamParser/include \
        $(MTK_PATH_SOURCE)/external/AudioParamParser

LOCAL_SHARED_LIBRARIES += libandroidicu

LOCAL_CFLAGS += -DSYS_IMPL

LOCAL_MODULE := libaudiocompensationfilter

LOCAL_MODULE_TAGS := optional

LOCAL_MULTILIB := both

include $(MTK_SHARED_LIBRARY)

############################################################################################################### Vendor
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
AudioCompFltCustParam.cpp

LOCAL_C_INCLUDES := \
    $(MTK_PATH_SOURCE)/external/nvram/libnvram \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/cgen/cfgfileinc

LOCAL_SHARED_LIBRARIES := \
    libcustom_nvram \
    libcutils \
    liblog \
    libutils

LOCAL_C_INCLUDES += \
        external/libxml2/include \
        external/icu/libandroidicu/include \
        $(JNI_H_INCLUDE) \
        $(MTK_PATH_SOURCE)/external/AudioParamParser/include \
        $(MTK_PATH_SOURCE)/external/AudioParamParser

LOCAL_SHARED_LIBRARIES += \
    libnvram

LOCAL_MODULE := libaudiocompensationfilter_vendor
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_MODULE_TAGS := optional

LOCAL_MULTILIB := both

include $(MTK_SHARED_LIBRARY)
############################## libaudiocompensationfilterc used by vendor only
include $(CLEAR_VARS)
LOCAL_C_INCLUDES:=  $(LOCAL_PATH)

LOCAL_SRC_FILES := \
    AudioCompFltCustParamc.c


LOCAL_C_INCLUDES := \
    $(MTK_PATH_SOURCE)/external/audiocustparam \
    $(MTK_PATH_SOURCE)/external/nvram/libnvram \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/cgen/cfgfileinc

LOCAL_SHARED_LIBRARIES := \
    libcustom_nvram \
    libnvram \
    libcutils \
    libutils \
    libaudiocustparam_vendor

LOCAL_C_INCLUDES += \
        external/libxml2/include \
        external/icu/libandroidicu/include \
        $(JNI_H_INCLUDE) \
        $(MTK_PATH_SOURCE)/external/AudioParamParser/include \
        $(MTK_PATH_SOURCE)/external/AudioParamParser

LOCAL_MODULE := libaudiocompensationfilterc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_MODULE_TAGS := optional

LOCAL_MULTILIB := both

LOCAL_LDLIBS := -llog
include $(MTK_SHARED_LIBRARY)
