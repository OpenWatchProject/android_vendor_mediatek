LOCAL_PATH := $(call my-dir)

############################################################################
# MAPI Header Libraries
############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE:= mdi_headers
LOCAL_EXPORT_C_INCLUDE_DIRS += $(LOCAL_PATH)/include
include $(BUILD_HEADER_LIBRARY)

############################################################################
# MAPI implementation: MDI redirector daemon
# Vendor executable
############################################################################
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := mdi_redirector
LOCAL_MODULE_OWNER := mtk
LOCAL_HEADER_LIBRARIES := mdi_headers

LOCAL_SRC_FILES := \
    src/mdi_redirector.cpp \
    src/CommandProcessor.cpp \
    src/NetworkSocketConnection.cpp \
    src/NetworkSocketManager.cpp \
    src/SocketListener.cpp \
    src/SocketConnection.cpp \
    src/DataQueue.cpp

LOCAL_SHARED_LIBRARIES := \
    liblog \
    vendor.mediatek.hardware.dmc@1.0 \
    vendor.mediatek.hardware.dmc@1.1 \
    libhidlbase \
    libhidltransport \
    libhwbinder \
    libutils

LOCAL_MODULE_TAGS := optional
LOCAL_INIT_RC := mdi_redirector.rc
include $(MTK_EXECUTABLE)

############################################################################
# MAPI implementation: MDI redirector control executable
# Vendor executable
############################################################################
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := mdi_redirector_ctrl
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES := \
    src/mdi_redirector_ctrl.cpp \
    src/SocketConnection.cpp \
    src/DataQueue.cpp

LOCAL_SHARED_LIBRARIES := libutils liblog libc++
#libcutils for property_get/set
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_HEADER_LIBRARIES := mdi_headers
LOCAL_MODULE_TAGS := optional
include $(MTK_EXECUTABLE)
