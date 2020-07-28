LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)
LOCAL_ARM_MODE:=arm
LOCAL_C_INCLUDES := $(MTK_PATH_SOURCE)/hardware/ccci/include \
	$(LOCAL_PATH)/platform

LOCAL_SRC_FILES:=$(call all-subdir-c-files,$(LOCAL_PATH))
LOCAL_MODULE:=ccci_fsd
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := 32
LOCAL_CFLAGS := -Wno-attributes
LOCAL_MODULE_TAGS := optional
LOCAL_INIT_RC := init.cccifsd.rc
include $(call all-makefiles-under,$(LOCAL_PATH))
include $(MTK_EXECUTABLE)

