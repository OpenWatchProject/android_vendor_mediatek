LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.mediatek.hardware.keyinstall@1.0-impl
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := mtk
ifeq ($(strip $(MTK_TEE_GP_SUPPORT)),yes)
    LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/trustzone/common/hal/source/trustlets/keyinstall/common/ca/public
else
    LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/trustzone/trustonic/source/trustlets/keyinstall/common/TlcKeyInstall/public
endif
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/meta/common/inc/
LOCAL_CFLAGS += -DDEBUG
LOCAL_SRC_FILES := \
    Keyinstall.cpp \

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libhidlbase \
    libhidltransport \
    libutils \
    libhardware \
    liburee_meta_drmkeyinstall \
    vendor.mediatek.hardware.keyinstall@1.0

LOCAL_STATIC_LIBRARIES := \
    vendor.mediatek.hardware.keyinstall@1.0-util

include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := vendor.mediatek.hardware.keyinstall@1.0-util
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := mtk
ifeq ($(strip $(MTK_TEE_GP_SUPPORT)),yes)
    LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/trustzone/common/hal/source/trustlets/keyinstall/common/ca/public
else
    LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/trustzone/trustonic/source/trustlets/keyinstall/common/TlcKeyInstall/public
endif
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/meta/common/inc/
LOCAL_SRC_FILES := \
    Keyinstall_util.cpp \

LOCAL_SHARED_LIBRARIES := \
    vendor.mediatek.hardware.keyinstall@1.0

include $(BUILD_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := vendor.mediatek.hardware.keyinstall@1.0-service
LOCAL_INIT_RC := vendor.mediatek.hardware.keyinstall@1.0-service.rc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES := \
    service.cpp

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libdl \
    libbase \
    libutils \
    libhardware \
    libhidlbase \
    libhidltransport \
    vendor.mediatek.hardware.keyinstall@1.0

include $(BUILD_EXECUTABLE)
