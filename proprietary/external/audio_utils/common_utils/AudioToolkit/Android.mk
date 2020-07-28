LOCAL_PATH := $(my-dir)

### ============================================================================
### AudioToolkit for system
### ============================================================================

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    AudioToolkit.cpp

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libutils \
    libaudioclient

LOCAL_C_INCLUDES := \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/ \
    $(MTK_PATH_SOURCE)/frameworks/av/media/libaudioclient/include/media/ \
    $(MTK_PATH_SOURCE)/system/core/base/include

LOCAL_CFLAGS := -DSYS_IMPL

LOCAL_MODULE := libaudiotoolkit
LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_OWNER := mtk

#ifeq ($(MTK_AUDIO_A64_SUPPORT),yes)
#LOCAL_MULTILIB := both
#else
#LOCAL_MULTILIB := 32
#endif

LOCAL_ARM_MODE := arm

include $(BUILD_SHARED_LIBRARY)

### ============================================================================
### AudioToolkit for vendor
### ============================================================================

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    AudioToolkit.cpp

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libutils

LOCAL_C_INCLUDES := \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/ \

LOCAL_MODULE := libaudiotoolkit_vendor
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#ifeq ($(MTK_AUDIO_A64_SUPPORT),yes)
#LOCAL_MULTILIB := both
#else
#LOCAL_MULTILIB := 32
#endif

LOCAL_ARM_MODE := arm

include $(BUILD_SHARED_LIBRARY)
