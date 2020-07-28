build_vilte =
ifeq ($(strip $(MSSI_MTK_VILTE_SUPPORT)),yes)
    build_vilte = yes
endif

ifeq ($(strip $(MSSI_MTK_VIWIFI_SUPPORT)),yes)
    build_vilte = yes
endif

ifdef build_vilte

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

PROCESS_SRC_PATH:=src
PROCESS_INC_PATH:=$(LOCAL_PATH)/inc

LOCAL_SRC_FILES:= \
    $(PROCESS_SRC_PATH)/main_vtservice.cpp

LOCAL_SHARED_LIBRARIES:= \
    libbinder \
    libutils \
    libcutils \
    liblog

LOCAL_C_INCLUDES := \
    $(TOP)/$(MTK_PATH_SOURCE)/frameworks/base/include\
    $(TOP)/system/core/include/private\
    $(PROCESS_INC_PATH)

LOCAL_SHARED_LIBRARIES += libmtk_vt_service

LOCAL_MODULE:= vtservice
LOCAL_INIT_RC := init.vtservice.rc
LOCAL_MULTILIB := 32

include $(MTK_EXECUTABLE)
endif
