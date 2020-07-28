LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS += -Werror -Wno-error=undefined-bool-conversion
LOCAL_CFLAGS += -Wall -Wextra
LOCAL_CFLAGS += -fexceptions

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH) \
    $(MTK_PATH_SOURCE)/external/aurisys/interface \
    $(MTK_PATH_SOURCE)/external/blisrc/blisrc32 \
    $(MTK_PATH_SOURCE)/external/shifter

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    libblisrc32_vendor \
    libmtkshifter_vendor

LOCAL_LDLIBS := \
    -llog

LOCAL_SRC_FILES := \
   audio_fmt_conv.c


LOCAL_MODULE := libaudiofmtconv
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_MULTILIB := both

include $(MTK_SHARED_LIBRARY)



