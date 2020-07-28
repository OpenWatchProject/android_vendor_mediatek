LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE := high_freq_sensor_test
LOCAL_SRC_FILES := \
    main.cpp 

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libutils 

LOCAL_SHARED_LIBRARIES += \
    libhfmanager 

LOCAL_C_INCLUDES += \
    $(MTK_PATH_SOURCE)/hardware/sensor/high_freq_sensor 

include $(BUILD_EXECUTABLE)
