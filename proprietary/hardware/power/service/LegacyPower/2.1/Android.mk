
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../util \
                    $(LOCAL_PATH)/../../../include \
                    $(LOCAL_PATH)/../../../config/common/intf_types

LOCAL_SRC_FILES := LegacyPower.cpp \
        ../../util/mi_util.cpp \
        ../../util/ptimer.cpp \
        ../../util/ports.cpp \
        ../../util/power_ipc.cpp \
        ../../util/powerc.cpp \
        ../../util/powerd_cmd.cpp

LOCAL_SHARED_LIBRARIES := liblog \
        libhardware \
        libhwbinder \
        libhidlbase \
        libhidltransport \
        libutils \
        libcutils \
        android.hardware.power@1.0 \
        android.hardware.power@1.1 \
        android.hardware.power@1.2 \
        vendor.mediatek.hardware.power@2.0 \
        vendor.mediatek.hardware.power@2.1

LOCAL_MODULE := vendor.mediatek.hardware.power@2.1-impl
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := mtk
include $(MTK_SHARED_LIBRARY)
