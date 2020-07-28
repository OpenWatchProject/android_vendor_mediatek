# =============================================================================
#
# MobiCore Android build components
#
# =============================================================================

LOCAL_PATH := $(call my-dir)

PLATFORM_VERSION_CODENAME ?= P
ifneq (P, $(word 1, $(sort P $(PLATFORM_VERSION_CODENAME)))) # PLATFORM_VERSION_CODENAME < P

# Registry Shared Library
# =============================================================================
include $(CLEAR_VARS)

LOCAL_MODULE := libMcRegistry
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS += -Wall -Wextra -Werror
LOCAL_CFLAGS += -DLOG_ANDROID

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_SHARED_LIBRARIES := libMcClient
ifeq ($(APP_PROJECT_PATH),)
LOCAL_SHARED_LIBRARIES += liblog
else
# Local build
LOCAL_LDLIBS := -llog
endif

LOCAL_SRC_FILES := \
	src/registry_log.cpp \
	src/Connection.cpp \
	src/Registry.cpp

LOCAL_HEADER_LIBRARIES += liblog_headers
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_EXPORT_C_INCLUDES)

include $(BUILD_SHARED_LIBRARY)
endif

# Daemon Application (statically linked to be compatible with recovery)
# =============================================================================
include $(CLEAR_VARS)

LOCAL_MODULE := mcDriverDaemon
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

LOCAL_INIT_RC := ../../rc/common/tee.rc
LOCAL_INIT_RC += ../../rc/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z )/trustonic.rc
LOCAL_CFLAGS += -DTBASE_API_LEVEL=9
LOCAL_CFLAGS += -Wall -Wextra -Werror
LOCAL_CFLAGS += -std=c++11
LOCAL_CFLAGS += -DLOG_ANDROID

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_STATIC_LIBRARIES := libMcClient_static
ifeq ($(APP_PROJECT_PATH),)
LOCAL_SHARED_LIBRARIES += \
	liblog libc

ifdef TRUSTONIC_ANDROID_LEGACY_SUPPORT

include external/stlport/libstlport.mk

LOCAL_C_INCLUDES += \
	external/stlport/stlport

LOCAL_STATIC_LIBRARIES += \
	libstlport_static

else # TRUSTONIC_ANDROID_LEGACY_SUPPORT

LOCAL_STATIC_LIBRARIES += \
	libc++_static

endif # !TRUSTONIC_ANDROID_LEGACY_SUPPORT

else # !NDK
# Local build
LOCAL_LDLIBS := -llog
endif # NDK

LOCAL_SRC_FILES := \
	src/daemon_log.cpp \
	src/Connection.cpp \
	src/MobiCoreDriverDaemon.cpp \
	src/SecureWorld.cpp \
	src/FSD2.cpp \
	src/RegistryUpdate.cpp \
	src/DebugSession.cpp \
	src/PrivateRegistry.cpp \
	src/RegistryServer.cpp \
	src/TuiStarter.cpp

LOCAL_HEADER_LIBRARIES += liblog_headers
#LOCAL_FORCE_STATIC_EXECUTABLE := true

include $(BUILD_EXECUTABLE)

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreClientLib_module))
