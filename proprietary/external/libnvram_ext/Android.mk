LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)
#LOCAL_ARM_MODE:=arm

LOCAL_SHARED_LIBRARIES:= libc libcutils libcustom_nvram libnvram_platform libbase liblog

ifneq ($(strip $(MTK_BASIC_PACKAGE)),yes)
  LOCAL_SHARED_LIBRARIES+=libnvram_sec
endif
LOCAL_SRC_FILES:= \
	libnvram.cpp

LOCAL_HEADER_LIBRARIES := \
	libnvram_headers \
	libfile_op_headers

# pass the include path of system/core/include/private/android_filesystem_config.h
LOCAL_C_INCLUDES += system/core/include/private
LOCAL_C_INCLUDES:= \
    system/core/include/private \
    system/core/fs_mgr/include_fstab/fstab

ifeq ($(strip $(MTK_BASIC_PACKAGE)),yes)
  LOCAL_STATIC_LIBRARIES += libfstab
  LOCAL_CFLAGS += -DMTK_BASIC_PACKAGE
else
  LOCAL_STATIC_LIBRARIES += libfstab
endif

LOCAL_MODULE:=libnvram
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
include $(MTK_SHARED_LIBRARY)

