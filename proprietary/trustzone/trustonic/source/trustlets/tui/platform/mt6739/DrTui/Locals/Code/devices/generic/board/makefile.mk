### Add source code files for C compiler here
INCLUDE_DIRS += Locals/Code/platforms/generic

SRC_C += \
    Locals/Code/devices/generic/board/board.c \
    Locals/Code/platforms/generic/lcd.c \
    Locals/Code/platforms/generic/fb.c \
    Locals/Code/peripherals/generic.c \
    Locals/Code/platforms/generic/i2c.c \
    Locals/Code/platforms/generic/gpio.c \
    Locals/Code/platforms/generic/led.c \
    Locals/Code/platforms/generic/dapc.c \
    Locals/Code/platforms/generic/mtk_log.c \
    Locals/Code/peripherals/gt1xx_driver.c \
    Locals/Code/peripherals/mtk_common_tpd.c \

include Locals/Code/platforms/cmdq/makefile.mk
include Locals/Code/platforms/display/$(ARCH_MTK_PLATFORM)/makefile.mk

ifeq ($(strip $(TARGET_BUILD_VARIANT)),user)
	ARMCC_COMPILATION_FLAGS += -DTUI_USER_BUILD
endif
