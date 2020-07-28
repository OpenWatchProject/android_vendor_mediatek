LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)
#LOCAL_ARM_MODE:=arm
LOCAL_SHARED_LIBRARIES:= libc libcutils libnvram libcustom_nvram liblog
LOCAL_SRC_FILES:= \
	libfile_op.c
LOCAL_C_INCLUDES:=\
	$(MTK_PATH_SOURCE)/external/nvram/libnvram \
	$(MTK_PATH_SOURCE)/external/nvram/libfile_op \
        $(TOP)/system/core/include/private \


LOCAL_MODULE:=libfile_op
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE:= false
include $(MTK_SHARED_LIBRARY)

