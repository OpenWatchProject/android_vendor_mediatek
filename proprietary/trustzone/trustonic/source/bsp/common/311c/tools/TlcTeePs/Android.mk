# =============================================================================
#
# Module: tee-ps
#
# =============================================================================
LOCAL_PATH	:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := tee-ps
LOCAL_CFLAGS := -DLOG_TAG=\"tee-ps\"
LOCAL_CFLAGS += -DLOG_ANDROID
LOCAL_CFLAGS += -std=c++11
LOCAL_LDLIBS := -llog

LOCAL_SRC_FILES := src/main.cpp \
	src/DebugInfo_kernel.cpp \
	src/DebugInfo_fc.cpp \
	src/DebugInfo_dbgfs.cpp \
	src/DebugInfo_rtm.cpp \
	src/DebugInfo.cpp \
	src/Utils.cpp

LOCAL_SHARED_LIBRARIES := libMcClient

include $(BUILD_EXECUTABLE)

$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreClientLib_module))
