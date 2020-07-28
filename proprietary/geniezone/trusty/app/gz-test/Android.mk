LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := gz_test.elf
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(GZ_APP_OUT)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_SRC_FILES += main.c \
                   manifest.c

LOCAL_MULTILIB := 32

LOCAL_C_INCLUDES := $(GZ_ROOT_DIR)/trusty/lib/interface/mtee/include/common/include/ \
                    $(GZ_ROOT_DIR)/trusty/lib/lib/mtee_serv/ \

LOCAL_CFLAGS :=

LOCAL_STATIC_LIBRARIES := libc-trusty \
                          libc.mod \
                          libmtee_serv \
                          libmtee_api \

include $(GZ_EXECUTABLE)

