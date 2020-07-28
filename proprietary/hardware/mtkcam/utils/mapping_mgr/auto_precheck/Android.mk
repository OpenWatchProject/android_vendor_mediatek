LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES += \
    recordevent.c 

#-----------------------------------------------------------

LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(LOCAL_PATH)

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libutils

#-----------------------------------------------------------

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := recordevent
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true

#-----------------------------------------------------------

include $(MTK_EXECUTABLE)

#-----------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_SRC_FILES += \
    replayevent.c 

#-----------------------------------------------------------

LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(LOCAL_PATH)

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libutils

#-----------------------------------------------------------

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := replayevent
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true

#-----------------------------------------------------------

include $(MTK_EXECUTABLE)

#-----------------------------------------------------------

