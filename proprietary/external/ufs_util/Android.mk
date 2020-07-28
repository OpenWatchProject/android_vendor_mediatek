LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES:= ufs_util.c
LOCAL_MODULE := ufs_util
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SHARED_LIBRARIES := libcutils libc
LOCAL_C_INCLUDES += \
    device/mediatek/common/kernel-headers/linux/scsi/ufs
include $(MTK_EXECUTABLE)
