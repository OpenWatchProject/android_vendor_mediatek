LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/widevine))
include $(CLEAR_VARS)
LOCAL_MODULE := oemcrypto_test
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_OWNER := widevine
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/data/nativetest
LOCAL_MODULE_TAGS := tests
LOCAL_SHARED_LIBRARIES := libbase libdl liblog libmedia_omx libstagefright_foundation libutils libz
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/oemcrypto_test
include $(BUILD_PREBUILT)
endif
