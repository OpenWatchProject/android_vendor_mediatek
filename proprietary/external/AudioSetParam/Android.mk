LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    AudioSetParam.cpp

LOCAL_C_INCLUDES := \
    $(TOPDIR)frameworks/native/include \
    $(TOPDIR)frameworks/av/include

LOCAL_CFLAGS += -Werror -Wno-error=undefined-bool-conversion -Wall -Wextra
LOCAL_CFLAGS += -fexceptions

LOCAL_SHARED_LIBRARIES := libutils libmedia libaudioclient liblog

LOCAL_MODULE := AudioSetParam
LOCAL_MODULE_OWNER := mtk

include $(BUILD_EXECUTABLE)

