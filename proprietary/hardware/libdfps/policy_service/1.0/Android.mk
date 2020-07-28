LOCAL_PATH := $(call my-dir)
#
# libfpspolicy-client
#
include $(CLEAR_VARS)
LOCAL_MODULE := libfpspolicy-client
LOCAL_CLANG := true
LOCAL_CFLAGS += -Wall -Werror -Wunused -Wunreachable-code
#LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES := \
        default/FpsPolicyClient.cpp \
        default/FpsInfoClient.cpp \
        default/FpsVsyncClient.cpp
LOCAL_SHARED_LIBRARIES := \
        vendor.mediatek.hardware.dfps@1.0 \
        libbase \
        libcutils \
        libfmq \
        libhardware \
        libhidlbase \
        libhidltransport \
        liblog \
        libsync \
        libutils
LOCAL_EXPORT_C_INCLUDE_DIRS := \
	$(LOCAL_PATH)/../../inc
include $(BUILD_STATIC_LIBRARY)


#
# vendor.mediatek.hardware.dfps@1.0-service
#
include $(CLEAR_VARS)
LOCAL_MODULE := vendor.mediatek.hardware.dfps@1.0-service
LOCAL_CLANG := true
LOCAL_CFLAGS += -Wall -Werror -Wunused -Wunreachable-code
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES := \
        default/service.cpp
LOCAL_INIT_RC := \
        default/vendor.mediatek.hardware.dfps@1.0-service.rc
LOCAL_STATIC_LIBRARIES := \
        libfpspolicy-client
LOCAL_SHARED_LIBRARIES := \
        vendor.mediatek.hardware.dfps@1.0 \
        libbase \
        libcutils \
        libfmq \
        libhardware \
        libhidlbase \
        libhidltransport \
        liblog \
        libsync \
        libutils
include $(BUILD_EXECUTABLE)


#
# vendor.mediatek.hardware.dfps@1.0-impl
#
include $(CLEAR_VARS)
LOCAL_MODULE := vendor.mediatek.hardware.dfps@1.0-impl
LOCAL_CLANG := true
LOCAL_CFLAGS += -Wall -Werror -Wunused -Wunreachable-code
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES := \
        default/DfpsHal.cpp
LOCAL_C_INCLUDES := \
        $(TOP)/$(MTK_ROOT)/hardware/libdfps/inc
LOCAL_STATIC_LIBRARIES := \
        libfpspolicy-client
LOCAL_SHARED_LIBRARIES := \
        vendor.mediatek.hardware.dfps@1.0 \
        libbase \
        libcutils \
        libfmq \
        libhardware \
        libhidlbase \
        libhidltransport \
        liblog \
        libsync \
        libutils
include $(BUILD_SHARED_LIBRARY)


#
# dfps.mtxxxx
#
include $(CLEAR_VARS)
LOCAL_MODULE := dfps.$(MTK_PLATFORM_DIR)
LOCAL_CLANG := true
LOCAL_CFLAGS += -Wall -Werror -Wunused -Wunreachable-code
LOCAL_CFLAGS += -DLOG_TAG=\"DynamicFpsPolicy\"
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := \
        dfps.cpp \
        FpsPolicyService.cpp \
        FpsPolicyTracker.cpp \
        string_def.cpp
LOCAL_C_INCLUDES:= \
        $(TOP)/$(MTK_PATH_SOURCE)/hardware/libdfps/inc \
        $(TOP)/$(MTK_PATH_SOURCE)/hardware/libgem/inc \
        $(TOP)/hardware/libhardware/include
LOCAL_SHARED_LIBRARIES := \
        libcutils \
        liblog \
        libutils
include $(MTK_SHARED_LIBRARY)


#
# libfpspolicy
#
include $(CLEAR_VARS)

include $(LOCAL_PATH)/BuildFpsPolicy.mk
LOCAL_MODULE := libfpspolicy
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SHARED_LIBRARIES += \
        vendor.mediatek.hardware.dfps@1.0

include $(BUILD_SHARED_LIBRARY)
