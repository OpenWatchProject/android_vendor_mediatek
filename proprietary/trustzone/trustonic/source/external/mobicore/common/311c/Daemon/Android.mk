# =============================================================================
#
# MobiCore Android build components
#
# =============================================================================

LOCAL_PATH := $(call my-dir)

# Registry Shared Library
# =============================================================================
include $(CLEAR_VARS)

LOCAL_MODULE := libMcRegistry
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

LOCAL_CFLAGS += -DLOG_TAG=\"McRegistry\"
LOCAL_CFLAGS += -Wall -Wextra
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
	src/Connection.cpp \
	src/Registry.cpp

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_EXPORT_C_INCLUDES)

include $(BUILD_SHARED_LIBRARY)

# Daemon Application (statically linked to be compatible with recovery)
# =============================================================================
include $(CLEAR_VARS)

LOCAL_MODULE := mcDriverDaemon
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true
LOCAL_INIT_RC := ../../rc/common/tee.rc \
	../../rc/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z )/trustonic.rc

LOCAL_CFLAGS += -DLOG_TAG=\"McDaemon\"
LOCAL_CFLAGS += -DTBASE_API_LEVEL=5
LOCAL_CFLAGS += -Wall -Wextra
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
	src/Connection.cpp \
	src/MobiCoreDriverDaemon.cpp \
	src/SecureWorld.cpp \
	src/FSD2.cpp \
	src/FSD2Wrapper.cpp \
	src/UpgradeStorage.cpp \
	src/DebugSession.cpp \
	src/PrivateRegistry.cpp \
	src/RegistryServer.cpp \
	src/TuiStarter.cpp

#LOCAL_FORCE_STATIC_EXECUTABLE := true

include $(BUILD_EXECUTABLE)

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreClientLib_module))
