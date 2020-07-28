#
# Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
# All Rights Reserved.
#

include config.mk

CONFIG_COMPILE_DEBUG := -g0

CONFIG_COMPILE_OPTIMIZATION := -O3

############################################SRC Config######################################
CONFIG_SRC_PATHS += \
         $(shell find Locals/Code/gf_lib/${GF_CHIP_SERIES}/irq -maxdepth 6 -type d) \
         $(shell find Locals/Code/gf_lib/${GF_CHIP_SERIES}/state -maxdepth 6 -type d) \
         $(shell find Locals/Code/gf_lib/$(GF_CHIP_SERIES)/hw -maxdepth 6 -type d | sed -e '/mtk/d' | sed -e '/qualcomm/d'  )

CONFIG_SRC_PATHS += ../public \
                    ../include

CONFIG_SRC_FILES := ${wildcard $(patsubst %, %/*.c, $(CONFIG_SRC_PATHS))} ${wildcard $(patsubst %, %/*.S, $(CONFIG_SRC_PATHS))}
CONFIG_SRC_FILES += \
        $(filter-out Locals/Code/gf_lib/common/gf_ta_user.c, $(wildcard $(patsubst %, %/*.c, $(shell find Locals/Code/gf_lib/common -maxdepth 6 -type d)))) \
        $(wildcard Locals/Code/gf_lib/$(GF_CHIP_SERIES)/*.c) \

ifeq ($(CPU_TYPE), qualcomm)
    CONFIG_SRC_FILES +=   $(foreach dir, Locals/Code/gf_lib/$(GF_CHIP_SERIES)/hw/qualcomm, $(wildcard $(dir)/*.c))
else
    CONFIG_SRC_FILES +=  $(foreach dir, Locals/Code/gf_lib/$(GF_CHIP_SERIES)/hw/mtk, $(wildcard $(dir)/*.c))
endif

ifeq ($(TARGET_MODE), release)
    CONFIG_SRC_FILES := $(filter-out Locals/Code/gf_lib/common/gf_ta_dump.c,$(CONFIG_SRC_FILES))
else
    CONFIG_SRC_FILES := $(filter-out Locals/Code/gf_lib/common/gf_ta_dump_stub.c,$(CONFIG_SRC_FILES))
endif

$(info CONFIG_SRC_FILES = $(CONFIG_SRC_FILES))

CONFIG_OUTPUT_NAME := gf_ta
CONFIG_OUTPUT_FILE := libgf_ta.a
CONFIG_OUTPUT_PATH := ../out/gf_ta/$(TARGET_MODE)/${CONFIG_ABI}

CONFIG_ALGO_INCLUDE_PATHS := $(shell find Locals/Code/gf_lib/algo -maxdepth 6 -type d)

CONFIG_INCLUDE_DIRS += $(CONFIG_SRC_PATHS) $(CONFIG_ALGO_INCLUDE_PATHS)
CONFIG_INCLUDE_DIRS += Locals/Code/platform/include \
                       Locals/Code/platform/$(TEE) \
                       Locals/Code \
                       Locals/Code/$(GF_CHIP_SERIES) \
                       Locals/Code/gf_lib/algo/common \
                       Locals/Code/public \
                       Locals/Code/gf_lib/include \
                       Locals/Code/gf_lib/public \
                       Locals/Code/gf_lib/$(GF_CHIP_SERIES)/base

ALGO_NAME := Locals/Code/gf_lib/$(TARGET_MODE)/$(GF_CHIP_SERIES)/$(TEE)/$(CONFIG_ABI)/libgf_ta_algo.a
ifeq ($(wildcard $(ALGO_NAME)), $(ALGO_NAME))
CONFIG_CUSTOMER_LIBS := $(ALGO_NAME)
else
CONFIG_CUSTOMER_LIBS := ../out/gf_ta/$(TARGET_MODE)/${CONFIG_ABI}/libgf_ta_algo.a
endif
