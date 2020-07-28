### Add source code files for C compiler here
INCLUDE_DIRS += Locals/Code/platforms/cmdq \
	../../../../../../../../bootable/bootloader/preloader/platform/$(ARCH_MTK_PLATFORM)/src/security/trustzone/inc \
	../../../../../../../../bootable/bootloader/preloader/platform/$(ARCH_MTK_PLATFORM)/src/core/inc \
	../../../../../../../../trustzone/common/hal/source/trustlets/secmem/common/drv_api_inc

SRC_C += \
    Locals/Code/platforms/cmdq/cmdq_sec_record.c \
    Locals/Code/platforms/cmdq/cmdq_sec_core.c \
    Locals/Code/platforms/cmdq/cmdq_sec_platform.c

