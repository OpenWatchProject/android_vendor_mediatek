LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  ratconfig.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/utils
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/property
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/log

LOCAL_SHARED_LIBRARIES:= \
  libmtkutils libmtkrillog libmtkcutils libmtkproperty

# If not declare LOCAL_MULTILIB, it will build both 32/64 bit lib.
# In c6m_rild 64bit project, all modules that use libmtkutils are
# 64bit, it is not necessary to build 32bit libmtkrilutils.
ifeq ($(MTK_RIL_MODE), c6m_1rild)
    LOCAL_MULTILIB := first
endif

LOCAL_MODULE:= libratconfig
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_EXPORT_C_INCLUDE_DIRS := \
  $(LOCAL_PATH)/include

include $(MTK_SHARED_LIBRARY)
