# =============================================================================
#
# Makefile responsible for:
# - building a test binary - tlcFoo
# - building the TLC library - libtlcFooLib
#
# =============================================================================

# Do not remove this - Android build needs the definition
LOCAL_PATH	:= $(call my-dir)

# =============================================================================
# binary tlcfoo
include $(CLEAR_VARS)

# Module name
LOCAL_MODULE	:= tuitest
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

# Add your source files here
LOCAL_SRC_FILES	+= main.cpp tlcfoo.cpp

LOCAL_C_INCLUDES +=\
    system/core/include \
    $(LOCAL_PATH)/../mobicore/MobiCoreDriverLib/ClientLib/public \
    $(LOCAL_PATH)/../mobicore/common/MobiCore/inc

LOCAL_SHARED_LIBRARIES := libMcClient liblog


include $(BUILD_EXECUTABLE)

# =============================================================================

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreDriverLib))
