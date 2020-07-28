LOCAL_ROOT_PATH := $(call my-dir)

MOBICORE_DIR_INC := $(LOCAL_ROOT_PATH)/../curl/include
include $(LOCAL_ROOT_PATH)/Code/Common/Android.mk
include $(LOCAL_ROOT_PATH)/Code/Android/app/jni/Android.mk
include $(LOCAL_ROOT_PATH)/Code/Android/lib/Android.mk
include $(LOCAL_ROOT_PATH)/Code/Android/app/Android.mk