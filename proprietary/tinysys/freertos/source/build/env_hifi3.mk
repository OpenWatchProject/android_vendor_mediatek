###########################################################
## Toolchain
###########################################################
ifeq (,$(ADSP_CROSS_COMPILE))
#  XT_TOOLS_VERSION ?= RG-2017.6-linux
   XT_TOOLS_VERSION ?= RI-2018.0-linux
  # Configurable XT related paths
  REL_ROOT_DIR := $(strip $(shell python -c 'import os; print(os.path.relpath("$(find_Android_root_dir)", "$(CURDIR)"))'))
  XT_TOOLS_BASE ?= $(REL_ROOT_DIR)/prebuilts/xcc/linux-x86/xtensa/RG-2017.6-linux
  XT_TOOLS_DIR  := $(XT_TOOLS_BASE)/$(XT_TOOLS_VERSION)/XtensaTools
  XT_HWREG_DIR  ?= $(THIRDPARTY_LIB_DIR)/hwcfg/$(XT_TOOLS_VERSION)

  XT_TOOLS_BIN_DIR   := $(XT_TOOLS_DIR)/bin
  ADSP_CROSS_COMPILE := $(XT_TOOLS_BIN_DIR)/
endif

CC      := $(ADSP_CROSS_COMPILE)xt-xcc
AS      := $(ADSP_CROSS_COMPILE)xt-xcc
AR      := $(ADSP_CROSS_COMPILE)xt-ar
OBJCOPY := $(ADSP_CROSS_COMPILE)xt-objcopy
OBJDUMP := $(ADSP_CROSS_COMPILE)xt-objdump
DUMPELF := $(ADSP_CROSS_COMPILE)xt-dumpelf
SIZE    := $(ADSP_CROSS_COMPILE)xt-size
STRIP   := $(ADSP_CROSS_COMPILE)xt-strip

HEX2BIN_UTIL := $(TOOLS_DIR)/hifi3tools/hex2bin_B.py

XTENSA_LICENSE_RETRY_COUNT := 20
XTENSA_LICENSE_RETRY_DELAY := 15
export XTENSA_LICENSE_RETRIES=$(XTENSA_LICENSE_RETRY_COUNT):$(XTENSA_LICENSE_RETRY_DELAY)

#XT_CORE := $(patsubst %-params,%,$(notdir $(shell $(CC) --show-config=core)))

ifeq (1,$(V))
# $(info $(TINYSYS_ADSP): XT_CORE=$(XT_CORE))
  $(info $(TINYSYS_ADSP): CC=$(CC))
  $(info $(TINYSYS_ADSP): AS=$(AS))
  $(info $(TINYSYS_ADSP): AR=$(AR))
  $(info $(TINYSYS_ADSP): OBJCOPY=$(OBJCOPY))
  $(info $(TINYSYS_ADSP): OBJDUMP=$(OBJDUMP))
  $(info $(TINYSYS_ADSP): DUMPELF=$(DUMPELF))
  $(info $(TINYSYS_ADSP): SIZE=$(SIZE))
  $(info $(TINYSYS_ADSP): STRIP=$(STRIP))
endif

COMMON_CFLAGS := -O2 -Os -g -gdwarf-2 -Wall -mno-coproc -mlongcalls -Werror \
                 -Wno-error=format --xtensa-core=$(PLATFORM)
CFLAGS        += $(COMMON_CFLAGS) -ffunction-sections -fdata-sections

TCM_RENAME_FLAGS := \
  -mrename-section-.text=.iram.text \
  -mrename-section-.literal=.iram.literal \
  -mrename-section-.data=.dram.data \
  -mrename-section-.rodata=.dram.rodata \
  -mrename-section-.bss=.dram.bss

# Define build type. Default to release
BUILD_TYPE ?= release

ifeq (debug,$(strip $(BUILD_TYPE)))
  CFLAGS += -DTINYSYS_DEBUG_BUILD
endif

###########################################################
## TCM specific resources
###########################################################
TCM_C_FILES += \
  $(RTOS_SRC_DIR)/portable/XCC/Xtensa/xtensa_intr.c \
  $(RTOS_SRC_DIR)/portable/XCC/Xtensa/port.c \
  $(RTOS_SRC_DIR)/portable/XCC/Xtensa/exception_handler.c

ifeq (2,$(TCM_LAYOUT_VERSION))
  TCM_C_FILES += \
    $(RTOS_SRC_DIR)/tasks.c \
    $(RTOS_SRC_DIR)/list.c \
    $(RTOS_SRC_DIR)/queue.c \
    $(RTOS_SRC_DIR)/timers.c \
    $(RTOS_SRC_DIR)/event_groups.c

ifeq (yes,$(CFG_WDT_SUPPORT))
  #TCM_C_FILES += $(SOURCE_DIR)/drivers/$(PROCESSOR)/$(PLATFORM)/wdt/src/wdt.c
  TCM_C_FILES += $(DRIVERS_PLATFORM_DIR_COMMON)/wdt/src/wdt.c
endif
ifeq (yes,$(CFG_BUS_MONITOR_SUPPORT))
  #TCM_C_FILES += $(SOURCE_DIR)/drivers/$(PROCESSOR)/$(PLATFORM)/bus_monitor/src/bus_monitor.c
  TCM_C_FILES += $(DRIVERS_PLATFORM_DIR_COMMON)/bus_monitor/src/bus_monitor.c
endif
endif

TCM_S_FILES += \
  $(RTOS_SRC_DIR)/portable/XCC/Xtensa/portasm.S \
  $(RTOS_SRC_DIR)/portable/XCC/Xtensa/xtensa_context.S \
  $(RTOS_SRC_DIR)/portable/XCC/Xtensa/xtensa_intr_asm.S \
  $(RTOS_SRC_DIR)/portable/XCC/Xtensa/xtensa_vectors.S \
  $(RTOS_SRC_DIR)/portable/XCC/Xtensa/EXCEPTION.S

###########################################################
## Common and image specific resources
###########################################################
INCLUDES += \
  $(XT_TOOLS_DIR)/lib/xcc/include \
  $(SOURCE_DIR)/kernel/service/HIFI3/include \
  $(RTOS_SRC_DIR)/include \
  $(RTOS_SRC_DIR)/portable/XCC/Xtensa

LDFLAGS += -ltrax -Wl,-u,vTaskDelay -Wl,--gc-sections -Wl,-wrap,printf -Wl,-Map -Wl,$($(PROCESSOR).MAP_FILE)
C_FILES += \
  $(filter-out $(RTOS_SRC_DIR)/string.c,$(wildcard $(RTOS_SRC_DIR)/*.c)) \
  $(RTOS_SRC_DIR)/portable/MemMang/heap_4.c \
  $(wildcard $(RTOS_SRC_DIR)/portable/XCC/Xtensa/*.c) \
  $(wildcard $(PLATFORM_DIR)/src/*.c) \
  $(SOURCE_DIR)/kernel/service/HIFI3/src/mtk_printf.c
C_FILES := $(filter-out $(TCM_C_FILES),$(C_FILES))

S_FILES += $(wildcard $(RTOS_SRC_DIR)/portable/XCC/Xtensa/*.S)
S_FILES := $(filter-out $(TCM_S_FILES),$(S_FILES))
