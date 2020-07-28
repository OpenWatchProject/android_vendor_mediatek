LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := main.c md_init_fsm.c time_clib_srv.c magic_pattern.c env_setting.c ccci_legacy.c ccci_log.c ccci_support_utils.c
#LOCAL_SRC_FILES := $(call all-subdir-c-files,$(LOCAL_PATH))

LOCAL_C_INCLUDES := \
	$(MTK_PATH_SOURCE)/external/nvram/libnvram \
	$(MTK_PATH_SOURCE)/external/libsysenv \
	$(MTK_PATH_SOURCE)/hardware/ccci/include \
	$(LOCAL_PATH)/platform

LOCAL_SHARED_LIBRARIES := libsysenv

LOCAL_CFLAGS += -Werror
LOCAL_MODULE := ccci_mdinit
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := 32

LOCAL_MODULE_TAGS := optional
LOCAL_INIT_RC := init.cccimdinit.rc

include $(call all-makefiles-under,$(LOCAL_PATH))
include $(MTK_EXECUTABLE)
