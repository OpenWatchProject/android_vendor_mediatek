LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.mediatek.hardware.keymanage@1.0-impl
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := mtk
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/trustzone/mtee/include/tz_cross/
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/meta/common/inc/
LOCAL_CFLAGS += -DDEBUG
LOCAL_SRC_FILES := \
    Keymanage.cpp \

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libhidlbase \
    libhidltransport \
    libutils \
    libhardware \
    liburee_meta_drmkeyinstall_v2 \
    vendor.mediatek.hardware.keymanage@1.0

LOCAL_STATIC_LIBRARIES := \
    vendor.mediatek.hardware.keymanage@1.0-util_vendor \
    liburee_meta_drmkey_if

include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := vendor.mediatek.hardware.keymanage@1.0-util_vendor
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := mtk
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/trustzone/mtee/include/tz_cross/
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/meta/common/inc/
LOCAL_SRC_FILES := \
    Keymanage_util.cpp \

LOCAL_SHARED_LIBRARIES := \
    vendor.mediatek.hardware.keymanage@1.0

LOCAL_STATIC_LIBRARIES := liburee_meta_drmkey_if

include $(BUILD_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := vendor.mediatek.hardware.keymanage@1.0-util
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := mtk
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/trustzone/mtee/include/tz_cross/
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/meta/common/inc/
LOCAL_SRC_FILES := \
    Keymanage_util.cpp \

LOCAL_SHARED_LIBRARIES := \
    vendor.mediatek.hardware.keymanage@1.0

#LOCAL_STATIC_LIBRARIES := liburee_meta_drmkey_if

include $(BUILD_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := vendor.mediatek.hardware.keymanage@1.0-service
LOCAL_INIT_RC := vendor.mediatek.hardware.keymanage@1.0-service.rc
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
    vendor.mediatek.hardware.keymanage@1.0

include $(BUILD_EXECUTABLE)
