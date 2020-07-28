### Add source code files for C compiler here
INCLUDE_DIRS += Locals/Code/platforms/cmdq \
	../../../../../../../../bootable/bootloader/preloader/platform/$(ARCH_MTK_PLATFORM)/src/security/trustzone/inc

SRC_C += \
    Locals/Code/platforms/cmdq/cmdq_sec_record.c \
    Locals/Code/platforms/cmdq/cmdq_sec_core.c \
    Locals/Code/platforms/cmdq/cmdq_sec_irqh.c \
    Locals/Code/platforms/cmdq/cmdq_sec_platform.c \
    Locals/Code/platforms/cmdq/cmdq_sec_sectrace.c \
