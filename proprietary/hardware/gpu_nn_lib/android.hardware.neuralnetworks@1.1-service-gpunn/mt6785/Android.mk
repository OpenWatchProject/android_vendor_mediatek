LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/hardware/gpu_nn))
include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.neuralnetworks@1.1-service-gpunn
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libbase libhidlbase libhidltransport libhidlmemory libdl libhardware liblog libtextclassifier_hash libutils android.hardware.neuralnetworks@1.0 android.hardware.neuralnetworks@1.1 android.hardware.neuralnetworks@1.2 android.hidl.allocator@1.0 android.hidl.memory@1.0 libOpenCL libnativewindow libui libneuropilot_hal_utils libarmnn libcmdl libcutils libged
LOCAL_INIT_RC := android.hardware.neuralnetworks@1.1-service-gpunn.rc
LOCAL_SRC_FILES := $(call get-prebuilt-src-arch,arm arm64)/android.hardware.neuralnetworks@1.1-service-gpunn
include $(BUILD_PREBUILT)
endif
