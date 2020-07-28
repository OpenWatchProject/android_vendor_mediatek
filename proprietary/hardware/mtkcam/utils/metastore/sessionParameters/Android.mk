LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libmtkcam_metastore.session
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_SRC_FILES := CaptureSessionParameters.cpp

LOCAL_HEADER_LIBRARIES := libmtkcam_headers

LOCAL_SHARED_LIBRARIES := \
	libmtkcam_ulog \
	libmtkcam_metadata

include $(MTK_STATIC_LIBRARY)