LOCAL_PATH := $(call my-dir)

ifeq ($(strip $(MTK_HDMI_SUPPORT)), yes)
include $(CLEAR_VARS)

ifeq ($(strip $(MTK_HDMI_SUPPORT)), yes)
  LOCAL_CFLAGS += -DMTK_HDMI_SUPPORT
endif
ifeq ($(strip $(MTK_HDMI_HDCP_SUPPORT)), yes)
  LOCAL_CFLAGS += -DMTK_HDMI_HDCP_SUPPORT
endif
ifeq ($(strip $(MTK_DRM_KEY_MNG_SUPPORT)), yes)
  ifeq ($(strip $(MTK_IN_HOUSE_TEE_SUPPORT)),yes)
    LOCAL_CFLAGS += -DMTK_DRM_KEY_MNG_SUPPORT
    LOCAL_CFLAGS += -DMTK_IN_HOUSE_TEE_SUPPORT
  endif
endif
ifeq ($(strip $(MTK_INTERNAL_HDMI_SUPPORT)), yes)
  LOCAL_CFLAGS += -DMTK_INTERNAL_HDMI_SUPPORT

endif
ifeq ($(strip $(MTK_MT8193_SUPPORT)), yes)
  LOCAL_CFLAGS += -DHDMI_MT8193_SUPPORT
endif

ifeq ($(strip $(MTK_ALPS_BOX_SUPPORT)), yes)
  LOCAL_CFLAGS += -DMTK_ALPS_BOX_SUPPORT
endif

LOCAL_MODULE := vendor.mediatek.hardware.hdmi@1.0-impl

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_RELATIVE_PATH := hw

ifeq ($(strip $(MTK_PLATFORM)), MT8173)
LOCAL_C_INCLUDES += \
    $(TOP)/device/mediatek/mt8173/kernel-headers
else ifeq ($(strip $(MTK_PLATFORM)), MT8167)
LOCAL_C_INCLUDES += \
    $(TOP)/device/mediatek/mt8167/kernel-headers
else ifeq ($(strip $(MTK_PLATFORM)), MT6735)
  ifeq ($(strip $(MTK_MT8193_SUPPORT)), yes)
    LOCAL_C_INCLUDES += \
        $(TOP)/device/mediatek/common/kernel-headers
  endif
else ifeq ($(strip $(MTK_PLATFORM)), MT6771)
LOCAL_C_INCLUDES += \
    $(TOP)/device/mediatek/common/kernel-headers
else ifeq ($(strip $(MTK_PLATFORM)), MT8163)
LOCAL_C_INCLUDES += \
    $(TOP)/device/mediatek/mt8163/kernel-headers
endif

LOCAL_CFLAGS += -DDEBUG

LOCAL_SRC_FILES := \
    MtkHdmiService.cpp \
    event/hdmi_event.cpp

LOCAL_SHARED_LIBRARIES := \
    libhidlbase \
    libhidltransport \
    libhwbinder \
    liblog \
    libutils \
    libcutils \
    vendor.mediatek.hardware.hdmi@1.0

ifeq ($(strip $(MTK_DRM_KEY_MNG_SUPPORT)), yes)
  ifeq ($(strip $(MTK_IN_HOUSE_TEE_SUPPORT)),yes)
    LOCAL_C_INCLUDES += \
        $(TOP)/vendor/mediatek/proprietary/external/trustzone/mtee/include/tz_cross \
        $(MTK_PATH_SOURCE)/hardware/keymanage/1.0 \
        $(MTK_PATH_SOURCE)/external/km_lib/drmkey/ \
        $(MTK_PATH_SOURCE)/hardware/meta/common/inc/
  endif
    LOCAL_SHARED_LIBRARIES += libcutils libnetutils libc
  ifeq ($(strip $(MTK_IN_HOUSE_TEE_SUPPORT)),yes)
    LOCAL_SHARED_LIBRARIES += libhidlbase libhidltransport
    LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.keymanage@1.0
    LOCAL_STATIC_LIBRARIES += vendor.mediatek.hardware.keymanage@1.0-util_vendor
  endif
endif

include $(MTK_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := vendor.mediatek.hardware.hdmi@1.0-service
LOCAL_INIT_RC := vendor.mediatek.hardware.hdmi@1.0-service.rc

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_SRC_FILES := \
    service.cpp

LOCAL_SHARED_LIBRARIES := \
    libhidlbase \
    libhidltransport \
    liblog \
    libutils \
    libcutils \
    libhardware \
    vendor.mediatek.hardware.hdmi@1.0

include $(MTK_EXECUTABLE)

include $(call all-makefiles-under,$(LOCAL_PATH))
endif
