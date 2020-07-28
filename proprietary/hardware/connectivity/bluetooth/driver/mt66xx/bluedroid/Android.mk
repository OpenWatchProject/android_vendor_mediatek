LOCAL_PATH := $(call my-dir)

###########################################################################
# MTK BT CHIP INIT LIBRARY FOR BLUEDROID
###########################################################################
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
  mtk.c \
  radiomgr.c \
  radiomod.c

LOCAL_C_INCLUDES := \
  system/bt/hci/include \
  $(MTK_GLOBAL_C_INCLUDES) \
  $(MTK_PATH_SOURCE)/external/nvram/libnvram \
  $(MTK_PATH_CUSTOM)/cgen/cfgfileinc \
  $(MTK_PATH_CUSTOM)/cgen/cfgdefault \
  $(MTK_PATH_CUSTOM)/cgen/inc \
  $(MTK_PATH_CUSTOM)/hal/bluetooth

ifeq ($(MTK_MERGE_INTERFACE_SUPPORT), yes)
LOCAL_CFLAGS += -D__MTK_MERGE_INTERFACE_SUPPORT__
endif

ifeq ($(MTK_CONSYS_ADIE), MT6631)
LOCAL_CFLAGS += -DMTK_CONSYS_ADIE_6631
endif

ifneq ($(filter userdebug eng,$(TARGET_BUILD_VARIANT)),)
LOCAL_CFLAGS += -DBD_ADDR_AUTOGEN
endif

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libbluetooth_mtk
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SHARED_LIBRARIES := liblog libcutils libnvram
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)

###########################################################################
# MTK BT DRIVER FOR BLUEDROID
###########################################################################
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
  bt_drv.c

LOCAL_C_INCLUDES := \
  system/bt/hci/include \
  $(MTK_GLOBAL_C_INCLUDES) \
  $(MTK_PATH_CUSTOM)/cgen/cfgfileinc

LOCAL_CFLAGS :=

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libbt-vendor
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SHARED_LIBRARIES := liblog libbluetooth_mtk
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)
