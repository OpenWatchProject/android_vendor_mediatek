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

MOBICORE_PROJECT_PATH := $(COMP_PATH_MobiCoreProjectPath)
LOG_WRAPPER := $(MOBICORE_PROJECT_PATH)/common/LogWrapper
# =============================================================================
# TLC Pinpad library - libtlcPinpad.a

include $(CLEAR_VARS)


# Module name (sets name of output binary / library)
LOCAL_MODULE	:= tlcPinpad
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

# Add your folders with header files here
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/public \
	$(LOG_WRAPPER)

# Add your source files here (relative paths)
LOCAL_SRC_FILES	+= \
	tlcPinpad.cpp

# Need the MobiCore client library
LOCAL_SHARED_LIBRARIES := libMcClient

include $(LOG_WRAPPER)/Android.mk

include $(BUILD_STATIC_LIBRARY)


# =============================================================================
# JNI module: TlcPinpad.so

include $(CLEAR_VARS)

LOCAL_MODULE	:= libTlcPinpad
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

# Add new source files here
LOCAL_SRC_FILES	+= jni/tlcPinpadJni.cpp

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/pictures \
	$(LOCAL_PATH)/public \
	$(LOG_WRAPPER)

LOCAL_STATIC_LIBRARIES := tlcPinpad
LOCAL_SHARED_LIBRARIES := libMcClient

include $(LOG_WRAPPER)/Android.mk

include $(BUILD_SHARED_LIBRARY)

# =============================================================================

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreDriverLib))
