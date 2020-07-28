ifeq ($(MTK_FM_TX_SUPPORT), yes)

LOCAL_PATH := $(call my-dir)
###############################################################################
# Define MTK FM Radio Chip solution
###############################################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	fmr_core.cpp \
	fmr_err.cpp \
	libfm_jni.cpp \
	common.cpp \

LOCAL_C_INCLUDES := $(JNI_H_INCLUDE) \
	frameworks/base/include/media

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libdl

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := libfmtxjni
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(BUILD_SHARED_LIBRARY)

endif
