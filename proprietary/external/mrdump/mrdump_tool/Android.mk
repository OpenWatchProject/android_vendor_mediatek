LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
	mrdump_log.c \
	mrdump_common.c \
	mrdump_status.c \
	mrdump_support_fiemap.c \
	mrdump_support_ext4.c \
	mrdump_support_f2fs.c \
	mrdump_support_mpart.c \
	mrdump_tool.c

LOCAL_C_INCLUDES := \
	vendor/mediatek/proprietary/external/libsysenv

LOCAL_MODULE := mrdump_tool
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libcutils libz liblog libsysenv
include $(MTK_EXECUTABLE)
