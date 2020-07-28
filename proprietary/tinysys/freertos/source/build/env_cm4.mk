###########################################################
## Toolchain
###########################################################
CC      := $(CROSS_COMPILE)gcc
CPP     := $(CROSS_COMPILE)cpp
OBJCOPY := $(CROSS_COMPILE)objcopy
SIZE    := $(CROSS_COMPILE)size
STRIP   := $(CROSS_COMPILE)strip

ifeq ($(CROSS_COMPILE),)
  # Apply default environment if CROSS_COMPILE is undefined
  TINYSYS_TOOLCHAIN_HOST   := linux-x86
  TINYSYS_TOOLCHAIN_STEM   := gcc-arm-none-eabi-4_8-2014q3
  TINYSYS_TOOLCHAIN_PREFIX ?= arm-none-eabi-

  $(info $(TINYSYS_SCP): Android root=$(find_Android_root_dir))
  $(info $(TINYSYS_SCP): CURDIR=$(CURDIR))
  REL_ROOT_DIR := $(strip $(shell python -c 'import os; print(os.path.relpath("$(find_Android_root_dir)", "$(CURDIR)"))'))
  $(info $(TINYSYS_SCP): REL_ROOT_DIR=$(REL_ROOT_DIR))
  TINYSYS_TOOLCHAIN_BIN_PATH := $(REL_ROOT_DIR)/prebuilts/gcc/$(TINYSYS_TOOLCHAIN_HOST)/arm/$(TINYSYS_TOOLCHAIN_STEM)/bin

  my_tinysys_cc := $(TINYSYS_TOOLCHAIN_BIN_PATH)/$(TINYSYS_TOOLCHAIN_PREFIX)gcc
  $(info $(TINYSYS_SCP): my_tinysys_cc=$(my_tinysys_cc))
  ifneq ($(wildcard $(my_tinysys_cc)),)
    CC      := $(my_tinysys_cc)
    CPP     := $(TINYSYS_TOOLCHAIN_BIN_PATH)/$(TINYSYS_TOOLCHAIN_PREFIX)g++
    OBJCOPY := $(TINYSYS_TOOLCHAIN_BIN_PATH)/$(TINYSYS_TOOLCHAIN_PREFIX)objcopy
    SIZE    := $(TINYSYS_TOOLCHAIN_BIN_PATH)/$(TINYSYS_TOOLCHAIN_PREFIX)size
    STRIP   := $(TINYSYS_TOOLCHAIN_BIN_PATH)/$(TINYSYS_TOOLCHAIN_PREFIX)strip
    READELF := $(TINYSYS_TOOLCHAIN_BIN_PATH)/$(TINYSYS_TOOLCHAIN_PREFIX)readelf
    CC_PREPROCESSOR := $(TINYSYS_TOOLCHAIN_BIN_PATH)/$(TINYSYS_TOOLCHAIN_PREFIX)cpp
  endif
endif # ifeq ($(CROSS_COMPILE),)

ifeq (1,$(V))
  $(info $(TINYSYS_SCP): CC=$(CC))
  $(info $(TINYSYS_SCP): CPP=$(CPP))
  $(info $(TINYSYS_SCP): OBJCOPY=$(OBJCOPY))
  $(info $(TINYSYS_SCP): SIZE=$(SIZE))
  $(info $(TINYSYS_SCP): STRIP=$(STRIP))
endif

###########################################################
## Check for valid float argument
##
## NOTE that you have to run make clean after changing
## these since hardfloat and softfloat are not binary
## compatible
###########################################################
ifneq ($(FLOAT_TYPE), hard)
  ifneq ($(FLOAT_TYPE), soft)
    override FLOAT_TYPE = hard
    #override FLOAT_TYPE = soft
  endif
endif

ifeq ($(FLOAT_TYPE), hard)
  FPUFLAGS = -fsingle-precision-constant -Wdouble-promotion
  FPUFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
  #CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
else
  FPUFLAGS = -msoft-float
endif

COMMON_CFLAGS := -g -Os -Wall -Werror -mlittle-endian -mthumb
CFLAGS += \
  -flto -ffunction-sections -fdata-sections -fno-builtin \
  -D_REENT_SMALL $(FPUFLAGS) -gdwarf-2 -std=c99

# Add to support program counter backtrace dump
CFLAGS += -funwind-tables

# Define build type. Default to release
BUILD_TYPE ?= release

# DO ID max length
DO_ID_MAX_LENGTH := 32

ifeq (1,$(V))
  $(info $(TINYSYS_SCP): BUILD_TYPE=$(BUILD_TYPE))
endif

ifeq (debug,$(strip $(BUILD_TYPE)))
  CFLAGS += -DTINYSYS_DEBUG_BUILD
endif

###########################################################
## LD flags
###########################################################
LDFLAGS += \
  $(FPUFLAGS) \
  --specs=nano.specs -lc -lnosys -lm \
  -nostartfiles --specs=rdimon.specs -lrdimon -Wl,--gc-sections

# Add to support printf wrapper function
LDFLAGS  += -Wl,-wrap,printf

###########################################################
## Processor-specific common instructions
###########################################################
PROCESSOR_FLAGS := -mcpu=cortex-m4
COMMON_CFLAGS   += $(PROCESSOR_FLAGS)
CFLAGS          += $(COMMON_CFLAGS)
LDFLAGS         += $(COMMON_CFLAGS)

###########################################################
## Common source codes
###########################################################
RTOS_FILES := \
  $(RTOS_SRC_DIR)/tasks.c \
  $(RTOS_SRC_DIR)/list.c \
  $(RTOS_SRC_DIR)/queue.c \
  $(RTOS_SRC_DIR)/timers.c \
  $(RTOS_SRC_DIR)/portable/MemMang/heap_4.c

###########################################################
## Include path
###########################################################
INCLUDES += \
  $(RTOS_SRC_DIR)/include \
  $(SOURCE_DIR)/$(APP_PATH) \
  $(SOURCE_DIR)/kernel/CMSIS/Include

###########################################################
## Processor-specific common instructions
###########################################################
RTOS_FILES += $(RTOS_SRC_DIR)/portable/GCC/ARM_CM4F/port.c
INCLUDES   += $(RTOS_SRC_DIR)/portable/GCC/ARM_CM4F

C_FILES += $(RTOS_FILES)

###########################################################
## Feature directives when used in DO
###########################################################
#FEATURE_COMMON_CFLAGS   := $(filter-out -funwind-tables,$(CFLAGS))
FEATURE_COMMON_CFLAGS   := $(CFLAGS)

###########################################################
## Common directives for DO
###########################################################
DO_PSEUDO_ENTRY    := __do_pseudo_startup_entry
DO_LINKER_SCRIPT   := $(DO_SERVICE_DIR)/dynamic_object.rel.ld
DO_COMMON_INCLUDES := $(DO_SERVICE_DIR)
# Files whose built objects will be linked in SCP image if DO is enabled
DO_SCP_C_FILES     := $(patsubst %,$(DO_SERVICE_DIR)/%, \
  do_service.c \
  dynamic_object.c \
  exidx_dynamic_object.c \
  verify.c)
DO_SCP_S_FILES     :=
# Source files whose built objects will be linked into each DO instance
DO_GENERIC_C_FILES  := $(DO_SERVICE_DIR)/pseudo_startup_entry.c
DO_GENERIC_S_FILES  :=
DO_STRIP_OPTIONS    := --strip-debug --strip-unneeded
DO_OBJCOPY_OPTIONS  := -O binary
#DO_COMMON_LDFLAGS   := $(filter-out -Wl$(COMMA)--gc-sections,$(LDFLAGS)) -Wl,-r -Wl,-T$(DO_LINKER_SCRIPT)
DO_COMMON_LDFLAGS   := -Wl,-r -Wl,--entry=$(DO_PSEUDO_ENTRY) -Wl,-T$(DO_LINKER_SCRIPT) $(LDFLAGS)
DO_ELFTOOLS_DIR     := $(TOOLS_DIR)/pyelftools
MKDOHEADER          := $(TOOLS_DIR)/mkdoheader
DO_PYTHON_ENV       := PYTHONDONTWRITEBYTECODE=1 PYTHONPATH=$(DO_ELFTOOLS_DIR)
