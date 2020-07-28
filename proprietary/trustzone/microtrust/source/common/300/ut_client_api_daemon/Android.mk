LOCAL_PATH := $(call my-dir)

#------------------libmtee----------------------------
include $(CLEAR_VARS)

LOCAL_MODULE := libmtee
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true
LOCAL_CFLAGS := -DBINARY_PREFIX=\"TEEC\"

#Pass default tee to open
ifeq ($(strip $(DEV_FEATURE_GP_SUPPORT)), yes)
LOCAL_CFLAGS+=-DDEFAULT_TEE_GPTEE
endif

LOCAL_LDFLAGS := -llog
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include

LOCAL_SRC_FILES := \
	IUtClientApiDaemon.cpp \
	teei_client_api.cpp \
	teec_trace.cpp \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
    system/core/include \
    hardware/libhardware/include \
    libnativehelper/include_jni \

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libhidlbase \
    libhwbinder \
    libimsg_log

LOCAL_SHARED_LIBRARIES += vendor.microtrust.hardware.capi@2.0.vendor
LOCAL_SHARED_LIBRARIES += libhidlmemory android.hidl.memory@1.0 android.hidl.allocator@1.0


include $(BUILD_SHARED_LIBRARY)
