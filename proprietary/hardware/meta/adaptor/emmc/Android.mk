LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := meta_clr_emmc.cpp
LOCAL_C_INCLUDES := $(MTK_PATH_SOURCE)/hardware/meta/common/inc
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/meta/adaptor/storageutil
LOCAL_C_INCLUDES += system/vold
LOCAL_C_INCLUDES += system/core/fs_mgr/include_fstab/fstab
LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_SHARED_LIBRARIES += libc
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_STATIC_LIBRARIES := libstorageutil
LOCAL_STATIC_LIBRARIES += libz
LOCAL_STATIC_LIBRARIES += libft
LOCAL_STATIC_LIBRARIES += libselinux

ifeq ($(MNTL_SUPPORT), yes)
LOCAL_CFLAGS += -DMNTL_SUPPORT
endif

LOCAL_MODULE := libmeta_clr_emmc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_PRELINK_MODULE := false
include $(MTK_STATIC_LIBRARY)
