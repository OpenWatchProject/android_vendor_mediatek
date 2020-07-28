LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

#-----------------------------------------------------------

#LOCAL_SRC_FILES += $(call all-cpp-files-under, .)

LOCAL_SRC_FILES := ./AdvCamService.cpp \
                   ./IAdvCamService.cpp


LOCAL_SHARED_LIBRARIES += \
    libui \
    liblog \
    libutils \
    libbinder \
    libcutils \
    libhardware \
    libcamera_client \
    libcamera_metadata \
    libgui \
    libhidlbase \
    libhidltransport \
    android.hardware.camera.common@1.0 \
    android.hardware.camera.provider@2.4 \
    android.hardware.camera.device@3.2 \

LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.camera.advcam@1.0

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/frameworks/av/services/mmsdk/include/
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/frameworks/av/services/mmsdk/include/advcam
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/frameworks/av/include/
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/frameworks-ext/av/include

################################################################################
LOCAL_MODULE:= libadvcamservice


include $(BUILD_SHARED_LIBRARY)
################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

