LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    ImageReader.cpp \
    Utils.cpp \
    DebugHelper.cpp \
    JpegCodec.cpp \

LOCAL_SHARED_LIBRARIES := \
    libandroid_runtime \
    libnativehelper \
    libutils \
    libmedia \
    libskia \
    liblog \
    libcutils \
    libgui \
    libstagefright \
    libstagefright_foundation \
    libcamera_client \
    libmtp \
    libpiex \
    libui \
    libion\
    libion_mtk\
    libdpframework\
    libmhalImageCodec\
    libJpgEncPipe\

LOCAL_C_INCLUDES += \
    external/piex/ \
    frameworks/av/media/libmedia \
    $(TOP)/mediatek/external/amr \
    frameworks/av/media/mtp \
    $(call include-path-for, libhardware)/hardware \
    $(PV_INCLUDES) \
    $(JNI_H_INCLUDE)

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/include

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/system/core/libion/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external

LOCAL_CFLAGS +=

LOCAL_MODULE:= libjni_jpegencoder
LOCAL_MULTILIB := both
#LOCAL_MULTILIB := 64

include $(BUILD_SHARED_LIBRARY)

