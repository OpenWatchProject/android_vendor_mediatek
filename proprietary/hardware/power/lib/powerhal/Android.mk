
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := perfservice.cpp \
    common.cpp \
    perfservice_smart.cpp \
    perfservice_xmlparse.cpp \
    utility_thermal.cpp \
    utility_consys.cpp \
    utility_fps.cpp \
    utility_vpu.cpp \
    utility_netd.cpp \
    utility_ux.c \
    utility_ril.c \
    utility_io.cpp \
    utility_sys.cpp

LOCAL_SHARED_LIBRARIES := libc libcutils libdl libui libutils liblog libexpat libtinyxml2\
    libhwbinder \
    libhidlbase \
    libhidltransport \
    libhardware \
    android.hardware.power@1.0 \
    android.hardware.power@1.1 \
    android.hardware.power@1.2 \
    android.hardware.power@1.3 \
    vendor.mediatek.hardware.power@2.0 \
    vendor.mediatek.hardware.netdagent@1.0

LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/power/config/common/intf_types \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/power/include \
    $(TOP/)$(MTK_PATH_SOURCE)/hardware/libgem/inc \
    external/tinyxml2

LOCAL_MODULE := libpowerhal
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(MTK_SHARED_LIBRARY)

