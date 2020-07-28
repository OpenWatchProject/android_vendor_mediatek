ifdef HIDL_V1_1

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.gnss@1.1-impl-mediatek
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SRC_FILES := \
    ThreadCreationWrapper.cpp \
    AGnss.cpp \
    AGnssRil.cpp \
    Gnss.cpp \
    GnssBatching.cpp \
    GnssDebug.cpp \
    GnssGeofencing.cpp \
    GnssMeasurement.cpp \
    GnssNavigationMessage.cpp \
    GnssNi.cpp \
    GnssXtra.cpp \
    GnssConfiguration.cpp \
    GnssUtils.cpp \

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/connectivity/gps/gps_hal/inc

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libhidlbase \
    libhidltransport \
    libutils \
    android.hardware.gnss@1.0 \
    android.hardware.gnss@1.1 \
    libhardware \

LOCAL_CFLAGS += -Werror

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE := android.hardware.gnss@1.1-service-mediatek
LOCAL_INIT_RC := android.hardware.gnss@1.1-service-mediatek.rc
LOCAL_SRC_FILES := \
    service.cpp \

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libdl \
    libbase \
    libutils \
    libhardware \
    libbinder \
    libhidlbase \
    libhidltransport \
    android.hardware.gnss@1.1 \

LOCAL_REQUIRED_MODULES := \
    android.hardware.gnss@1.1-impl-mediatek

include $(MTK_EXECUTABLE)

endif
