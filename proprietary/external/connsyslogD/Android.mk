LOCAL_PATH := $(call my-dir)

common_include_files := \
       external/libcxx/include \
       bionic \
       bionic/libstdc++/include \
      
common_src_files := \
            ConsysLogger.cpp \
            LogWriter.cpp \
            Engine.cpp \
            Utils.cpp \
            PropParser.cpp \
            SocketConnection.cpp \
            LogRecycle.cpp \
            LogFile.cpp \
            Connsys_Interface/LogInterface.cpp\
            ClearLog.cpp\
            LogTransfer.cpp

common_shared_lib := libutils libcutils libselinux  libc++ liblog

common_local_cflags := -Wno-unused-parameter


include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_MODULE := connsyslogger
#LOCAL_PROPRIETARY_MODULE := true
#LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_INIT_RC := consyslogger.rc

LOCAL_C_INCLUDES += $(common_include_files)
LOCAL_SRC_FILES := $(common_src_files)
LOCAL_MULTILIB := 32

LOCAL_SHARED_LIBRARIES := $(common_shared_lib)
include $(MTK_EXECUTABLE)


