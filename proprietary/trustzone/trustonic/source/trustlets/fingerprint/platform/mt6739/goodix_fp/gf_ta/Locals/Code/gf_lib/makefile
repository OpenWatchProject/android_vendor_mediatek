#
# Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
# All Rights Reserved.
#

include Locals/Code/gf_lib/sub_config.mk
include base.mk

ifeq ($(TEE), trustone)
CONFIG_SRC_FILES += \
    Locals/Code/platform/$(TEE)/gf_ta_trustone.c
endif

CONFIG_SRC_OBJS = $(patsubst %.S, %.o, $(patsubst %.c, %.o, $(CONFIG_SRC_FILES)))

CONFIG_AR_FILE := $(CONFIG_OUTPUT_FILE)

all:$(CONFIG_OUTPUT_FILE)
	mkdir -p  ${CONFIG_OUTPUT_PATH}
	cp $(CONFIG_OUTPUT_FILE)  ${CONFIG_OUTPUT_PATH}

clean:
	rm -rf  $(CONFIG_SRC_OBJS)
	rm -rf  $(CONFIG_OUTPUT_FILE)

include rules.mk

