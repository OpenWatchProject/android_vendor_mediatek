TRUSTONIC_LEGACY_BUILD =

ifdef TRUSTONIC_ANDROID_8
TRUSTONIC_LEGACY_BUILD = yes
endif

ifneq ($(APP_PROJECT_PATH),)
TRUSTONIC_LEGACY_BUILD = yes
endif

LOCAL_PATH := $(call my-dir)

ifdef TRUSTONIC_LEGACY_BUILD

# Registry Shared Library
# =============================================================================
include $(CLEAR_VARS)

LOCAL_MODULE := libMcRegistry
LOCAL_PROPRIETARY_MODULE := true

LOCAL_CFLAGS += -Wall -Wextra -Werror
LOCAL_CFLAGS += -DLOG_ANDROID

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_SHARED_LIBRARIES := libMcClient
# Local build
LOCAL_LDLIBS := -llog

LOCAL_SRC_FILES := \
	src/registry_log.cpp \
	src/Connection.cpp \
	src/Registry.cpp

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_EXPORT_C_INCLUDES)

include $(BUILD_SHARED_LIBRARY)

endif # TRUSTONIC_LEGACY_BUILD

# Daemon Application (statically linked to be compatible with recovery)
# =============================================================================
include $(CLEAR_VARS)

LOCAL_MODULE := mcDriverDaemon
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

LOCAL_INIT_RC := ../../rc/common/tee.rc
LOCAL_INIT_RC += ../../rc/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z )/trustonic.rc
LOCAL_CFLAGS += -DTBASE_API_LEVEL=5
LOCAL_CFLAGS += -Wall -Wextra -Werror
LOCAL_CFLAGS += -std=c++11
LOCAL_CFLAGS += -DLOG_ANDROID

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_STATIC_LIBRARIES := libMcClient_static
ifeq ($(APP_PROJECT_PATH),)
LOCAL_STATIC_LIBRARIES += \
	libc++_static

LOCAL_SHARED_LIBRARIES := \
	liblog

else # !NDK
# Local build
LOCAL_LDLIBS := -llog

LOCAL_LDLIBS  += -ldl
LOCAL_LDFLAGS += -nodefaultlibs

endif # NDK

LOCAL_SRC_FILES := \
	src/daemon_log.cpp \
	src/Connection.cpp \
	src/MobiCoreDriverDaemon.cpp \
	src/SecureWorld.cpp \
	src/FSD2.cpp \
	src/DebugSession.cpp \
	src/EndorsementInstaller.cpp \
	src/PrivateRegistry.cpp \
	src/RegistryServer.cpp \
	src/TuiStarter.cpp

include $(BUILD_EXECUTABLE)

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreClientLib_module))
