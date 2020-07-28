LOCAL_PATH := $(call my-dir)

# Client lib

include $(CLEAR_VARS)

LOCAL_MODULE := libMcClient
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

LOCAL_CFLAGS := -fvisibility=hidden
LOCAL_CFLAGS += -DTBASE_API_LEVEL=5
LOCAL_CFLAGS += -Wall -Wextra
LOCAL_CFLAGS += -std=c++11

LOCAL_CFLAGS += -DWITHOUT_PROXY

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/include/GP

ifeq ($(APP_PROJECT_PATH),)
LOCAL_SHARED_LIBRARIES := \
	liblog

else # !NDK
LOCAL_LDLIBS := -llog

endif # NDK

LOCAL_SRC_FILES := \
	src/common_client.cpp \
	src/driver_client.cpp \
	src/mc_client_api.cpp \
	src/tee_client_api.cpp

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_EXPORT_C_INCLUDES)

include $(BUILD_SHARED_LIBRARY)

# Static version of the client lib for recovery

include $(CLEAR_VARS)

LOCAL_MODULE := libMcClient_static
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

LOCAL_CFLAGS := -fvisibility=hidden
LOCAL_CFLAGS += -DTBASE_API_LEVEL=5
LOCAL_CFLAGS += -Wall -Wextra
LOCAL_CFLAGS += -std=c++11
LOCAL_CFLAGS += -DWITHOUT_PROXY

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/include/GP


LOCAL_SRC_FILES := \
	src/common_client.cpp \
	src/driver_client.cpp \
	src/mc_client_api.cpp \
	src/tee_client_api.cpp

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include $(LOCAL_PATH)/include/GP
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_EXPORT_C_INCLUDES)

include $(BUILD_STATIC_LIBRARY)

# =============================================================================

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_AndroidProtoBuf))

# =============================================================================
# Module: libTEECommon, alias of libMcClient.so
# =============================================================================

include $(CLEAR_VARS)

LOCAL_MODULE := libTEECommon
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

LOCAL_CFLAGS := -fvisibility=hidden
LOCAL_CFLAGS += -DTBASE_API_LEVEL=5
LOCAL_CFLAGS += -Wall -Wextra
LOCAL_CFLAGS += -std=c++11

LOCAL_CFLAGS += -DWITHOUT_PROXY

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/include/GP

ifeq ($(APP_PROJECT_PATH),)
LOCAL_SHARED_LIBRARIES := \
	liblog

else # !NDK
LOCAL_LDLIBS := -llog

endif # NDK

LOCAL_SRC_FILES := \
	src/common_client.cpp \
	src/driver_client.cpp \
	src/mc_client_api.cpp \
	src/tee_client_api.cpp

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_EXPORT_C_INCLUDES)

include $(BUILD_SHARED_LIBRARY)

