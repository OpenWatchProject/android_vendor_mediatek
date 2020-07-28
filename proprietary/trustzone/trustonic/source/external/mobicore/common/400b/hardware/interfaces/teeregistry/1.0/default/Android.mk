# =============================================================================
#
# MobiCore Android build components
#
# =============================================================================

LOCAL_PATH := $(call my-dir)

PLATFORM_VERSION_CODENAME ?= P
ifeq (P, $(word 1, $(sort P $(PLATFORM_VERSION_CODENAME)))) # PLATFORM_VERSION_CODENAME < P

# Registry Shared Library
# =============================================================================
include $(CLEAR_VARS)

LOCAL_MODULE := libMcRegistry
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS += -Wall -Wextra -Werror
LOCAL_CFLAGS += -DLOG_ANDROID

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/trustzone/trustonic/source/external/mobicore/common/$(TRUSTONIC_TEE_VERSION)/ClientLib/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/trustzone/trustonic/source/external/mobicore/common/$(TRUSTONIC_TEE_VERSION)/ClientLib/include/GP

LOCAL_SHARED_LIBRARIES := liblog
LOCAL_SHARED_LIBRARIES += libhidlbase
LOCAL_SHARED_LIBRARIES += libhidlmemory
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += android.hidl.allocator@1.0
LOCAL_SHARED_LIBRARIES += android.hidl.memory@1.0
LOCAL_SHARED_LIBRARIES += vendor.trustonic.teeregistry@1.0

LOCAL_SRC_FILES := \
	Registry.cpp

LOCAL_HEADER_LIBRARIES += liblog_headers
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_EXPORT_C_INCLUDES)

include $(BUILD_SHARED_LIBRARY)
endif

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreClientLib_module))
