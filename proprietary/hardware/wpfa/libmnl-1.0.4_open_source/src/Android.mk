LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES :=  socket.c callback.c nlmsg.c attr.c

LOCAL_HEADER_LIBRARIES := libmnlHeaders libmnlConfigHeaders
# libmnlConfigCapHeaders

LOCAL_CFLAGS := "-Wall"  "-Werror" "-Wno-error=pointer-sign"  "-Wno-sign-compare" "-Wno-unused-parameter" "-Wno-pointer-arith"

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE:=libmnetlink_v104

#include $(BUILD_SHARED_LIBRARY)
include $(MTK_SHARED_LIBRARY)