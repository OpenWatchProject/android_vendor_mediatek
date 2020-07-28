
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../util \
                    $(LOCAL_PATH)/../../../include \
                    $(LOCAL_PATH)/../../../config/common/intf_types

LOCAL_SRC_FILES := MtkPower.cpp \
        MtkPerf.cpp \
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
        vendor.mediatek.hardware.mtkpower@1.0

LOCAL_MODULE := vendor.mediatek.hardware.mtkpower@1.0-impl
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := mtk
include $(MTK_SHARED_LIBRARY)
