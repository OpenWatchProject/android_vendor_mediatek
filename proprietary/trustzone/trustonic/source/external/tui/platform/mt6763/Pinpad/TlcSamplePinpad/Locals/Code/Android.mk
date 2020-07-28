# =============================================================================
#
# Makefile responsible for:
# - building the Java JNI wrapper - TlcPinpad.so
# - building a test binary - pinpadTest
# - building the TLC library - libtlcPinpad.a (used by test binary + Java lib)
#
# =============================================================================

# Do not remove this - Android build needs the definition
LOCAL_PATH	:= $(call my-dir)

# =============================================================================
# TLC Pinpad library - libtlcPinpad.a

include $(CLEAR_VARS)

# Module name (sets name of output binary / library)
LOCAL_MODULE	:= tlcPinpad
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

# Enable logging to logcat per default
LOCAL_CFLAGS += -DLOG_ANDROID
LOCAL_LDLIBS += -llog

# Add your folders with header files here
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/src/ \
	vendor/mediatek/proprietary/trustzone/trustonic/source/bsp/common/$(TRUSTONIC_TEE_VERSION)/t-sdk/TlcSdk/Out/Public/GP \
	vendor/mediatek/proprietary/trustzone/trustonic/source/trustlets/tui/platform/$(MTK_PLATFORM_DIR)/TlSamplePinpad/Locals/Code/public


# Add your source files here (relative paths)
LOCAL_SRC_FILES	+= \
	src/tlcPinpad.cpp

LOCAL_SHARED_LIBRARIES := libMcClient

include $(BUILD_STATIC_LIBRARY)


# =============================================================================
# JNI module: TlcPinpad.so

include $(CLEAR_VARS)

LOCAL_MODULE	:= libTlcPinpad
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

# Enable logging to logcat per default
LOCAL_CFLAGS += -DLOG_ANDROID
LOCAL_CFLAGS += -DTBASE_API_LEVEL=9
LOCAL_LDLIBS += -llog

# Add new source files here
LOCAL_SRC_FILES	+= src/jni/tlcPinpadJni.cpp

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/src/ \
	$(LOCAL_PATH)/src/pictures \
	vendor/mediatek/proprietary/trustzone/trustonic/source/bsp/common/$(TRUSTONIC_TEE_VERSION)/t-sdk/TlcSdk/Out/Public \
	vendor/mediatek/proprietary/trustzone/trustonic/source/bsp/common/$(TRUSTONIC_TEE_VERSION)/t-sdk/TlcSdk/Out/Public/GP \
	vendor/mediatek/proprietary/trustzone/trustonic/source/trustlets/tui/platform/$(MTK_PLATFORM_DIR)/TlSamplePinpad/Locals/Code/public

LOCAL_STATIC_LIBRARIES := tlcPinpad
LOCAL_SHARED_LIBRARIES := libMcClient
LOCAL_HEADER_LIBRARIES := jni_headers

include $(BUILD_SHARED_LIBRARY)

# =============================================================================

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_TeeClient_module))
