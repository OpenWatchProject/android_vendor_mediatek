LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := trusty-lib-prebuilt

MODULE_SRCS += \
        $(LOCAL_DIR)/dummy.c \


MODULE_DEPS += \
        lib/libc-trusty \
        lib/msee_fwk \
        lib/gp_client \
        lib/libstdc++-trusty \
        lib/libcxx-mtee \
        lib/libm-mtee \
        lib/storage


ifeq (true,$(call TOBOOL,$(WITH_MTEE)))
MODULE_DEPS += \
        lib/mtee_api \
        lib/mtee_serv
endif

include make/module.mk

