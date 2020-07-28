LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := multi_threads_malloc_test.c
LOCAL_STATIC_LIBRARIES :=
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_C_INCLUDES :=


LOCAL_MODULE := mtm_test
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
ifeq ($(TARGET_IS_64_BIT),true)
LOCAL_MODULE_STEM_32 := mtm_test
LOCAL_MODULE_STEM_64 := mtm_test64
LOCAL_MULTILIB := both
endif

include $(MTK_EXECUTABLE)
