# Android NDK Native APIs
# https://developer.android.com/ndk/guides/stable_apis.html

$(info ANDROID_VERSION = $(ANDROID_VERSION))
$(info ANDROID_VENDOR_PATH = $(ANDROID_VENDOR_PATH))
$(info MICROTRUST_TEE_SUPPORT = $(MICROTRUST_TEE_SUPPORT))

APP_OPTIM := debug
APP_ABI := armeabi-v7a arm64-v8a
APP_PLATFORM := android-21
APP_BUILD_SCRIPT := Android.mk
APP_STL := c++_static
APP_CFLAGS += -DMICROTRUST_TEE_DEBUG_BUILD
APP_LDFLAGS += -llog -Wl,--allow-shlib-undefined
