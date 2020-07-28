LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)
#LOCAL_ARM_MODE:=arm
LOCAL_SHARED_LIBRARIES:= libc \
  libnvram \
  liblog \

LOCAL_STATIC_LIBRARIES += libft

LOCAL_SRC_FILES:=Meta_APEditor_Para.c
LOCAL_C_INCLUDES:= \
	$(MTK_PATH_SOURCE)/hardware/meta/common/inc \
    $(MTK_PATH_SOURCE)/external/nvram/libnvram \

LOCAL_MODULE:=libmeta_apeditor
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_PRELINK_MODULE:=false
include $(MTK_STATIC_LIBRARY)


