#
# Modules to be compiled into lk.bin
#
MODULES += \
	lib/sm \
	lib/trusty \
	lib/memlog \
	source/trusty-kernel/mtsmcall \
	source/trusty-kernel/uart_switch \
	source/trusty-kernel/libvmm \

ifeq (true,$(call TOBOOL,$(WITH_MTEE)))
MODULES += \
	source/trusty-kernel/mtee/common/src/sys

ifeq (1, $(WITH_MTEE_KERNEL_UNITTEST))
MODULES += \
	source/trusty-kernel/test_mtee \
	source/trusty-kernel/test_kta_mem
endif
endif

ifeq (1, $(WITH_KERNEL_UNITTEST))
MODULES += \
	source/trusty-kernel/unittest
endif

#
# user tasks to be compiled into lk.bin
#

# prebuilt
TRUSTY_PREBUILT_USER_TASKS :=

# compiled from source
TRUSTY_ALL_USER_TASKS := \
	source/trusty-user/app/timer \

ifeq (true,$(call TOBOOL,$(WITH_MTEE)))
TRUSTY_ALL_USER_TASKS += \
	source/trusty-user/app/mtee_kernel_service/main \
	source/trusty-user/app/echo/main

ifeq (1, $(WITH_MTEE_USER_UNITTEST))
TRUSTY_ALL_USER_TASKS += \
	source/trusty-user/app/mtee_test2 \
	source/trusty-user/app/fps_go_sample \

TRUSTY_ALL_USER_TASKS += source/trusty-user/app/ipc-unittest/srv
endif
endif

# user prebuilt library

EXTRA_BUILDRULES += device/arm/mediatek/build/user-lib-prebuilt.mk

