LOCAL_PATH := $(call my-dir)
LOCAL_PATH_ROOT := $(LOCAL_PATH)

############################################################################
# MDMI interface libraries
############################################################################
include $(call all-subdir-makefiles)
LOCAL_PATH := $(LOCAL_PATH_ROOT)

############################################################################
# MDMI header libraries
############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE:= mdmi_public_headers
LOCAL_EXPORT_C_INCLUDE_DIRS += $(LOCAL_PATH)/include/public
include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:= mdmi_redirector_headers
LOCAL_EXPORT_C_INCLUDE_DIRS += $(LOCAL_PATH)/include/test
include $(BUILD_HEADER_LIBRARY)

############################################################################
# MDMI implementation: MDMI redirector daemon, a test tool to
# verify MDMI functions.
# Vendor executable
############################################################################
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := mdmi_redirector
LOCAL_MODULE_OWNER := mtk
LOCAL_HEADER_LIBRARIES := \
    mdmi_redirector_headers \
    mdmi_public_headers

LOCAL_SRC_FILES := \
    src/test/mdmi_redirector.cpp \
    src/test/CommandProcessor.cpp \
    src/test/NetworkSocketConnection.cpp \
    src/test/NetworkSocketManager.cpp \
    src/test/SocketListener.cpp \
    src/test/SocketConnection.cpp \
    src/test/DataQueue.cpp

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libutils

LOCAL_MODULE_TAGS := optional
LOCAL_INIT_RC := mdmi_redirector.rc
include $(MTK_EXECUTABLE)

############################################################################
# MDMI implementation: MDMI redirector control executable, a test tool to
# verify MDMI functions.
# Vendor executable
############################################################################
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := mdmi_redirector_ctrl
LOCAL_MODULE_OWNER := mtk

LOCAL_SRC_FILES := \
    src/test/mdmi_redirector_ctrl.cpp \
    src/test/SocketConnection.cpp \
    src/test/DataQueue.cpp

LOCAL_SHARED_LIBRARIES := libutils liblog libc++
#libcutils for property_get/set
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_HEADER_LIBRARIES := \
    mdmi_redirector_headers \
    mdmi_public_headers

LOCAL_MODULE_TAGS := optional
include $(MTK_EXECUTABLE)