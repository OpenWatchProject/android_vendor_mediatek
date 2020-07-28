LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE      := storage.elf
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(GZ_APP_OUT)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_SRC_FILES += \
        block_allocator.c \
        block_cache.c \
        block_device_tipc.c \
        block_mac.c \
        block_map.c \
        block_set.c \
        block_tree.c \
        client_tipc.c \
        crypt.c \
        file.c \
        ipc.c \
        main.c \
        manifest.c \
        proxy.c \
        rpmb.c \
        super.c \
        tipc_ns.c \
        transaction.c \

LOCAL_MULTILIB := 32

LOCAL_STATIC_LIBRARIES := libc-trusty \
                          libc.mod \
                          libhwkey \
                          librng \
                          libopenssl_stubs \
                          libmtee_boringssl \

include $(GZ_EXECUTABLE)
