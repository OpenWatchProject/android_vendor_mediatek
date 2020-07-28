LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)
#LOCAL_ARM_MODE:=arm
LOCAL_SHARED_LIBRARIES:= libc libcutils libcustom_nvram liblog

ifeq ($(strip $(MTK_INTERNAL_LOG_ENABLE)),yes)
    LOCAL_CFLAGS += -DMTK_INTERNAL_LOG_ENABLE
endif

LOCAL_SRC_FILES:= \
   	libnvram_sec.c
		
#	libnvram.c
LOCAL_C_INCLUDES:= \
      $(MTK_PATH_SOURCE)/external/nvram/libfile_op \
      $(MTK_PATH_SOURCE)/external/nvram/libnvram

LOCAL_MODULE:=libnvram_sec
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE:= false
include $(MTK_SHARED_LIBRARY)

