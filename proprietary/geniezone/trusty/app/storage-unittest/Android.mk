LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE      := storage-unittest.elf
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(GZ_APP_OUT)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_SRC_FILES += \
        manifest.c \
        main.c \

LOCAL_MULTILIB := 32

LOCAL_STATIC_LIBRARIES := libc-trusty \
                          libc.mod \
                          libmtee_storage \

include $(GZ_EXECUTABLE)
