# =============================================================================
#
# Makefile responsible for:
# - building two test binary - tlcmem_example dumpsecmemusage
# - building the TLC library - libsec_mem.so
#
# =============================================================================

ifneq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)

# Do not remove this - Android build needs the definition
LOCAL_PATH	:= $(call my-dir)

# =============================================================================
# library tlcutilsLib
include $(CLEAR_VARS)

# Module name
LOCAL_MODULE	:= libsec_mem
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

# Add your folders with header files here
LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/inc \
    $(LOCAL_PATH)/public

# Add your source files here (relative paths)
LOCAL_SRC_FILES	+= tlcsecmem.cpp
# Need the MobiCore client library
LOCAL_SHARED_LIBRARIES := libMcClient
LOCAL_SHARED_LIBRARIES += liblog

include $(BUILD_SHARED_LIBRARY)

# =============================================================================
# binary tlcutils
ifeq ($(strip $(TARGET_BUILD_VARIANT)), eng)
include $(CLEAR_VARS)

# Module name
LOCAL_MODULE	:= tlcmem_example
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/public \
    $(LOCAL_PATH)/inc

# Add your source files here
LOCAL_SRC_FILES	+= main.cpp

LOCAL_SHARED_LIBRARIES := libMcClient
LOCAL_SHARED_LIBRARIES += libsec_mem
LOCAL_SHARED_LIBRARIES += liblog

include $(BUILD_EXECUTABLE)
endif

# =============================================================================
# binary tlcutils
ifeq ($(strip $(TARGET_BUILD_VARIANT)), eng)
include $(CLEAR_VARS)

# Module name
LOCAL_MODULE	:= dumpsecmemusage
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/public \
    $(LOCAL_PATH)/inc

# Add your source files here
LOCAL_SRC_FILES	+= dumpsecmemusage.cpp

LOCAL_SHARED_LIBRARIES := libMcClient
LOCAL_SHARED_LIBRARIES += libsec_mem
LOCAL_SHARED_LIBRARIES += liblog

include $(BUILD_EXECUTABLE)
endif

endif # MTK_TEE_GP_SUPPORT != yes
