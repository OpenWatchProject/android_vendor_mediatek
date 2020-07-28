LOCAL_PATH :=$(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := MtkWLParser.cpp

LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/hardware/perfservice/perfservicenative/ \
                    $(TOP)/$(MTK_ROOT)/hardware/libomx/video/MtkWLParser/include/ \


LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    libdl \
    liblog \
    libexpat \
    libbinder \
    libwlservice \


LOCAL_MODULE := libwlparser
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(MTK_SHARED_LIBRARY)

#
# unit test
#
#include $(CLEAR_VARS)

#LOCAL_SRC_FILES:= ../../test/main.cpp \

#LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/hardware/libomx/video/MtkWLParser/include/ \
#                    $(TOP)/$(MTK_ROOT)/hardware/libomx/video/MtkWLParser/service/parserservice/include

#LOCAL_SHARED_LIBRARIES += \
#    libcutils \
#    libdl \
#    liblog

#LOCAL_MODULE := wl_test
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#LOCAL_MULTILIB := 32
#include $(MTK_EXECUTABLE)
