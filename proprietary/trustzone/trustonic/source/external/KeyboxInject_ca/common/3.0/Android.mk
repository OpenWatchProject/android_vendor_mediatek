# =============================================================================
#
# =============================================================================

# Do not remove this - Android build needs the definition
LOCAL_PATH := $(call my-dir)

# =============================================================================
# library libKeyboxInject_ca
include $(CLEAR_VARS)

# Module name
LOCAL_MODULE := libkmsetkey_ca.trustonic

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional

LOCAL_CPPFLAGS := -Wall
LOCAL_CPPFLAGS += -Wextra
LOCAL_CPPFLAGS += -Werror
LOCAL_CPPFLAGS += -std=c++11

# Enable logging to logcat per default
LOCAL_CPPFLAGS += -DLOG_ANDROID

# Store attestation key in persist partition
ifeq ($(KEYMASTER_RPMB),yes)
LOCAL_CPPFLAGS += -DRPMB_SUPPORT=1
else
LOCAL_CPPFLAGS += -DRPMB_SUPPORT=0
endif

LOCAL_LDLIBS := -llog

# Add your folders with header files here
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/../../../keymaster/common/$(TRUSTONIC_TEE_VERSION)/$(KEYMASTER_VERSION)/include

# Add your source files here (relative paths)
LOCAL_SRC_FILES	+= src/KeyboxInject.cpp

# Need the MobiCore client library
LOCAL_SHARED_LIBRARIES := libMcClient

LOCAL_HEADER_LIBRARIES := libhardware_headers

include $(BUILD_STATIC_LIBRARY)

# =============================================================================
# binary KeyboxInject_ca
include $(CLEAR_VARS)

# Module name
LOCAL_MODULE := kmsetkey_ca.trustonic

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional

# Enable logging to logcat per default
LOCAL_CFLAGS += -DLOG_ANDROID

# Store attestation key in RPMB or persist partition
ifeq ($(KEYMASTER_RPMB),yes)
LOCAL_CPPFLAGS += -DRPMB_SUPPORT=1
else
LOCAL_CPPFLAGS += -DRPMB_SUPPORT=0
endif

LOCAL_LDLIBS += -llog

# Add your folders with header files here
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/include

# Add your source files here (relative paths)
LOCAL_SRC_FILES	+= src/main.cpp

LOCAL_STATIC_LIBRARIES := libkmsetkey_ca.trustonic
LOCAL_SHARED_LIBRARIES := libMcClient
LOCAL_HEADER_LIBRARIES := libhardware_headers

include $(BUILD_EXECUTABLE)

# =============================================================================

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_TeeClient_module))
$(call import-module,$(COMP_PATH_MobiCoreClientLib_module))
