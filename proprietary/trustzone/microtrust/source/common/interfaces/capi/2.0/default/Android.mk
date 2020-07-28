LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := vendor.microtrust.hardware.capi@2.0-service
LOCAL_INIT_RC := vendor.microtrust.hardware.capi@2.0-service.rc
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
  system/core/include \
  hardware/libhardware/include \

LOCAL_CFLAGS := -DBINARY_PREFIX=\"TEEC\"

#Pass default tee to open
ifeq ($(strip $(DEV_FEATURE_GP_SUPPORT)), yes)
LOCAL_CFLAGS+=-DDEFAULT_TEE_GPTEE
endif


LOCAL_SRC_FILES := \
    service.cpp \
    ClientApiDevice.cpp \
    UtClientApiDaemonProxy.cpp \
    teec_trace.cpp
#MyParcel.cpp

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    libhidlbase \
    libhidltransport \
    libhidlmemory \
    android.hidl.memory@1.0 \
    vendor.microtrust.hardware.capi@2.0.vendor \
    libimsg_log \
    liblog \
    libc \
    libhwbinder

include $(BUILD_EXECUTABLE)
