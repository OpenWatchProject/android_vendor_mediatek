
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := Profile_test.cpp
LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    liblog
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../include
LOCAL_MODULE := MtkVencLib_ut
LOCAL_MULTILIB := 32
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := RoiInfoConfig_test.cpp
LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    liblog
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../include
LOCAL_MODULE := MtkVenc_RoiInfo_ut
LOCAL_MULTILIB := 32
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(BUILD_EXECUTABLE)
