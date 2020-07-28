LOCAL_PATH := $(call my-dir)

ifeq ($(strip $(TRUSTONIC_TEE_SUPPORT)),yes)

###############################################################################
# RDA LIBRARY
###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := librda
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_SRC_FILES_arm := lib/librda.a
LOCAL_SRC_FILES_arm64 := lib64/librda.a
LOCAL_MODULE_SUFFIX := .a
LOCAL_SYSTEM_SHARED_LIBRARIES := libc liblog libcutils
LOCAL_SHARED_LIBRARIES := libMcClient
LOCAL_MULTILIB := both
include $(BUILD_PREBUILT)

###############################################################################
# RDA DAEMON
###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := rda
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES := rda.c
LOCAL_C_INCLUDES += inc
LOCAL_MODULE_TAGS := optional
LOCAL_SYSTEM_SHARED_LIBRARIES := libc liblog libcutils
LOCAL_SHARED_LIBRARIES := libMcClient
LOCAL_STATIC_LIBRARIES += librda
LOCAL_MODULE_CLASS := EXECUTABLES
include $(MTK_EXECUTABLE)

endif

###############################################################################
# IMAGE AUTH LIBRARY
###############################################################################

include $(CLEAR_VARS)
LOCAL_MODULE := libsecdl
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES_arm := lib/libsecdl.so
LOCAL_SRC_FILES_arm64 := lib64/libsecdl.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_MULTILIB := both
include $(BUILD_PREBUILT)

