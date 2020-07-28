LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    hwmsen_custom.c 

LOCAL_EXPORT_C_INCLUDE_DIRS := \
        $(LOCAL_PATH)/ 

LOCAL_HEADER_LIBRARIES += libandroid_sensor_headers
LOCAL_SHARED_LIBRARIES += libhardware
LOCAL_MODULE:= libsensor_custom
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
include $(MTK_SHARED_LIBRARY)
