LOCAL_PATH:= $(call my-dir)

commonSources:= \
    tinyxml.cpp \
    tinyxmlparser.cpp \
    tinyxmlerror.cpp \
    tinystr.cpp

# =====================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    $(commonSources)
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/tinyxml

LOCAL_MODULE:= libmtktinyxml

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

include $(MTK_SHARED_LIBRARY)
