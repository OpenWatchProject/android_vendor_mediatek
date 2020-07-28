LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/ncc/android-nn-driver))
include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.neuralnetworks@1.1-service-neuron-ann
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_SUFFIX := .1-service-neuron-ann
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/bin/hw
LOCAL_MODULE_STEM := android.hardware.neuralnetworks@1
LOCAL_SHARED_LIBRARIES := libfmq libhardware libhidlbase libhidlmemory libnativewindow libneuralnetworks libtextclassifier_hash libneuropilot_hal_utils android.hardware.neuralnetworks@1.0 android.hardware.neuralnetworks@1.1 vendor.mediatek.hardware.neuropilot@1.0 android.hidl.allocator@1.0 android.hidl.memory@1.0 android.hardware.neuralnetworks@1.2 vendor.mediatek.hardware.neuropilot@1.1 libbase libutils liblog libion libion_mtk libcutils libneuron_platform libc++ libc libm libdl
LOCAL_INIT_RC := android.hardware.neuralnetworks@1.1-service-neuron-ann.rc
LOCAL_MULTILIB := 64
LOCAL_SRC_FILES_64 := arm64/android.hardware.neuralnetworks@1.1-service-neuron-ann
include $(BUILD_PREBUILT)
endif
