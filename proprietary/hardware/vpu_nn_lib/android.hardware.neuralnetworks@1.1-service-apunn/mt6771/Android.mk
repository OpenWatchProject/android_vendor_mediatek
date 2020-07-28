LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/hardware/vpu_nn))
include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.neuralnetworks@1.1-service-apunn
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SHARED_LIBRARIES := libbase libdl libhardware libhidlbase libhidlmemory libhidltransport libnativewindow libtextclassifier_hash liblog libutils android.hardware.neuralnetworks@1.0 android.hardware.neuralnetworks@1.1 android.hardware.neuralnetworks@1.2 android.hidl.allocator@1.0 android.hidl.memory@1.0 libion libcutils libc++ libion_mtk libvpu libneuropilot_hal_utils
LOCAL_INIT_RC := android.hardware.neuralnetworks@1.1-service-apunn.rc
LOCAL_SRC_FILES := $(call get-prebuilt-src-arch,arm arm64)/android.hardware.neuralnetworks@1.1-service-apunn
include $(BUILD_PREBUILT)
endif
