#for hidl lib
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
    Mms.cpp

LOCAL_C_INCLUDES := \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/dpframework/include \
    $(TOP)/$(MTK_PATH_SOURCE)/external/libion_mtk \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include \
    $(TOP)/system/core/libion/include/ion \
    $(TOP)/system/core/libsync/include \
    $(TOP)/device/mediatek/common/kernel-headers/linux \
    $(TOP)/system/core/base/include

LOCAL_SHARED_LIBRARIES := \
    libhidlbase \
    libhidltransport \
    libhwbinder \
    liblog \
    libutils \
    libcutils \
    libion \
    libion_mtk \
    libhardware \
    libdpframework \
    libgralloc_extra \
    libsync \
    vendor.mediatek.hardware.mms@1.0 \
    vendor.mediatek.hardware.mms@1.1 \
    vendor.mediatek.hardware.mms@1.2 \
    vendor.mediatek.hardware.mms@1.3 \
    vendor.mediatek.hardware.mms@1.4 \
    vendor.mediatek.hardware.mms@1.5

LOCAL_CFLAGS += -DMMS_SUPPORT_JPG_ENC

LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/include/enc

LOCAL_SHARED_LIBRARIES += \
    libJpgEncPipe

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6580 mt6735 mt6737t mt6735m mt6737m mt6753 mt6755 mt6750 mt6757 mt8167))
#ifeq ($(MTK_M4U_SUPPORT), yes)
    LOCAL_CFLAGS += -DMTK_M4U_SUPPORT

    LOCAL_SHARED_LIBRARIES += \
        libm4u
endif

LOCAL_MODULE := vendor.mediatek.hardware.mms@1.5-impl
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_RELATIVE_PATH := hw
include $(MTK_SHARED_LIBRARY)
