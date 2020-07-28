LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)
#LOCAL_ARM_MODE:=arm
LOCAL_SHARED_LIBRARIES:= libc liblog libcustom_nvram libnvram
LOCAL_SRC_FILES:= \
      libnvram_daemon_callback.c
LOCAL_C_INCLUDES:= \
      $(MTK_PATH_SOURCE)/external/nvram/libfile_op \
      $(MTK_PATH_SOURCE)/external/nvram/libnvram \
      $(MTK_PATH_SOURCE)/external/mtd_util \
LOCAL_STATIC_LIBRARIES := libmtd_util

LOCAL_MODULE:=libnvram_daemon_callback
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
include $(MTK_SHARED_LIBRARY)

