ifneq ($(strip $(MSSI_MTK_TC1_COMMON_SERVICE)), yes)
LOCAL_ROOT_PATH:= $(call my-dir)

include $(LOCAL_ROOT_PATH)/host/Android.mk
include $(LOCAL_ROOT_PATH)/portability/Android.mk
include $(LOCAL_ROOT_PATH)/tests/Android.mk
include $(LOCAL_ROOT_PATH)/testscat/Android.mk
#include $(LOCAL_ROOT_PATH)/feature/pluginroot/Android.mk
#include $(LOCAL_ROOT_PATH)/feature/mode/vsdof/Android.mk
#include $(LOCAL_ROOT_PATH)/feature/mode/slowmotion/Android.mk
#include $(LOCAL_ROOT_PATH)/feature/mode/facebeauty/Android.mk
#include $(LOCAL_ROOT_PATH)/feature/mode/vfacebeauty/Android.mk

#CamPostAlgo
include $(LOCAL_ROOT_PATH)/camerapostalgo/Android.mk
endif
