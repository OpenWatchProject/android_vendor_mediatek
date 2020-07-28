# =============================================================================
#
# Module: TUI JNI library
#
# =============================================================================

# Do not remove this - Android build needs the definition
LOCAL_PATH	:= $(call my-dir)

MOBICORE_PROJECT_PATH := $(COMP_PATH_MobiCoreProjectPath)
$(info ****TUI*** $(MOBICORE_PROJECT_PATH))
#LOG_WRAPPER := $(COMP_PATH_Logwrapper)
LOG_WRAPPER := $(MOBICORE_PROJECT_PATH)/common/LogWrapper

include $(CLEAR_VARS)

# Module name (sets name of output binary / library)
LOCAL_MODULE := libTui
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

# Add your source files here (relative paths)
LOCAL_SRC_FILES += tlcTui.cpp \
	tlcTuiJni.cpp

# Add your folders with header files here (absolute paths)
LOCAL_C_INCLUDES += \
	$(LOG_WRAPPER)

# Undefine NDEBUG to enable LOG_I/LOG_W in Logwrapper
LOCAL_CFLAGS += -UNDEBUG

include $(LOG_WRAPPER)/Android.mk

include $(BUILD_SHARED_LIBRARY)

# =============================================================================

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
