LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= LanunchPPPoe.c
LOCAL_CFLAGS := -Werror=format
LOCAL_MODULE:= launchpppoe
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_C_INCLUDES := system/core/include/netutils/ system/core/include/cutils/
LOCAL_SHARED_LIBRARIES := libcutils libnetutils liblog
ifneq (,$(filter userdebug eng,$(TARGET_BUILD_VARIANT)))
LOCAL_CFLAGS += -DINIT_ENG_BUILD
endif
include $(MTK_EXECUTABLE)
