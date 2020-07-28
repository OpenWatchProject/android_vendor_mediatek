LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# securecamera_test - Unit tests of secure camera device
LOCAL_MODULE := securecamera_test
LOCAL_MODULE_TAGS := tests
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_SRC_FILES := \
	tests/InteractiveTest.cpp \
	tests/AImageReaderTest.cpp \
	tests/Camera2VendorTest.cpp \
	tests/MapperAndAllocatorTest.cpp \
	tests/ImageBufferHeapTest.cpp \
	tests/main.cpp

LOCAL_HEADER_LIBRARIES := \
	libnativebase_headers \
	libui_headers \
	libmtkcam_headers \
	libgralloc_extra_headers

LOCAL_STATIC_LIBRARIES := \
	libarect \
	libsecurecameratest \
	android.hardware.camera.common@1.0-helper

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libsync \
	libutils \
	libcutils \
	libnativewindow \
	libcamera_metadata \
	libmediandk \
	libcamera2ndk_vendor \
	libhidlbase \
	android.hidl.token@1.0-utils \
	android.hardware.graphics.mapper@2.0 \
	android.hardware.graphics.allocator@2.0 \
	android.hardware.graphics.bufferqueue@1.0 \
	vendor.mediatek.hardware.gpu@1.0 \
	libmtkcam_ulog \
	libmtkcam_imgbuf

# NOTE: set ENABLE_SECURE_IMAGE_DATA_TEST to false for
#       testing normal camera flow with Camera HAL3 impl.
#       This compiler flag is a handy option for
#       comparing the difference between normal and secure flow.
LOCAL_CPPFLAGS := -DENABLE_SECURE_IMAGE_DATA_TEST=true

# We depends LL-NDK API AImageReader_getWindowNativeHandle of the Vendor NDK,
# so add __ANDROID_VNDK__ to enable this LL-NDK API.
#
# Reference:
# https://source.android.com/devices/architecture/vndk/enabling#managing-headers
LOCAL_CPPFLAGS += -D__ANDROID_VNDK__

# MTK TEE
ifeq ($(strip $(MTK_ENABLE_GENIEZONE)), yes)
$(info MTK TEE is enabled)
LOCAL_CFLAGS += -DMTK_TEE_SUPPORTED=true
else
LOCAL_CFLAGS += -DMTK_TEE_SUPPORTED=false
endif

include $(BUILD_NATIVE_TEST)
