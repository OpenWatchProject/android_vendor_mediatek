LOCAL_PATH :=$(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := MtkWLService.cpp IMtkWLService.cpp

LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/hardware/libomx/video/MtkWLParser/include/ \
    $(LOCAL_PATH)/include

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    liblog \
    libexpat \
    libbinder \

LOCAL_EXPORT_C_INCLUDE_DIRS := \
        $(LOCAL_PATH)/include

LOCAL_EXPORT_SHARED_LIBRARY_HEADERS := libbinder

LOCAL_MODULE := libwlservice
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(MTK_SHARED_LIBRARY)
