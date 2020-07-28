################################################################################

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------

#-----------------------------------------------------------
LOCAL_SRC_FILES += main.cpp \
                   TestSensorProvider.cpp

#-----------------------------------------------------------

LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include
LOCAL_C_INCLUDES += $(TOP)/frameworks/hardware/interfaces/sensorservice/libsensorndkbridge
LOCAL_C_INCLUDES += $(TOP)/hardware/interfaces/sensors/1.0/default/include
LOCAL_C_INCLUDES += $(TOP)/external/googletest/googletest/include
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include/mtkcam
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES +=
#
LOCAL_STATIC_LIBRARIES += libgtest

#-----------------------------------------------------------
# vector
LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libutils \

LOCAL_SHARED_LIBRARIES += \
        libmtkcam_stdutils \
        android.frameworks.sensorservice@1.0 \
        android.hardware.sensors@1.0 \
        libhidlbase \
        libhidltransport \
        libsensorndkbridge \
        libcam.utils.sensorprovider \

#-----------------------------------------------------------
LOCAL_MODULE := sensorprovider.test
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#-----------------------------------------------------------
include $(MTK_EXECUTABLE)

