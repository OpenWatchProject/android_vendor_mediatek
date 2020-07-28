LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := perfctl.cpp
LOCAL_LDLIBS := -llog
LOCAL_SHARED_LIBRARIES := libc libcutils libdl libui libutils libexpat
LOCAL_MODULE := libperfctl_vendor
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(MTK_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := perfctl.cpp
LOCAL_LDLIBS := -llog
LOCAL_SHARED_LIBRARIES := libc libcutils libdl libgui libui libutils libexpat
LOCAL_MODULE := libperfctl
LOCAL_MODULE_OWNER := mtk
include $(MTK_SHARED_LIBRARY)


