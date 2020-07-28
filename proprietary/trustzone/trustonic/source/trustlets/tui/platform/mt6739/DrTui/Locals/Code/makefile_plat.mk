################################################################################
#
# <t-base TUI Driver
#
################################################################################

DRIVER_KEYFILE := Locals/Build/pairVendorTltSig.pem

#-------------------------------------------------------------------------------
# Files and include paths
#-------------------------------------------------------------------------------
DEVICE ?= GENERIC
MODE ?= Debug

include Locals/Code/makefile.*.mk

### Add include path here
INCLUDE_DIRS += \
    Locals/Code/inc \
    Locals/Code/public \
    Locals/Code/devices/$(DEVICE_PATH) \
    Locals/Code/platforms \

# For EMI MPU APIs
LOCAL_TRUSTZONE_PROTECT_BSP_PATH=../../../../../../../../../../../vendor/mediatek/proprietary/trustzone/trustonic/source/trustlets
INCLUDE_DIRS += $(LOCAL_TRUSTZONE_PROTECT_BSP_PATH)/mpu/platform/${ARCH_MTK_PLATFORM}/Drmpu/Locals/Code/public \

### Add library source code files for C compiler here
SRC_LIB_C += \

### Add source code files for C++ compiler here
SRC_CPP += #nothing

### Add source code files for C compiler here
SRC_C += \
	Locals/Code/device.c \

### Add source code files for assembler here
SRC_S += # nothing

include Locals/Code/devices/$(DEVICE_PATH)/makefile.mk
# ************************************************************************************************
# TUI Core
DRTUICORE_LIB := Out/Bin/$(MODE)/drTuiCore.a
EXTRA_LIBS += $(DRTUICORE_LIB)

# ************************************************************************************************

# output binary name without path or extension
OUTPUT_NAME := drTui

USE_PLAT_EMI_MPU ?= YES

include Locals/Code/driverTui.mk

ARMCC_COMPILATION_FLAGS += -DDRIVER_ID=$(DRIVER_ID)

# MTK options
ARMCC_COMPILATION_FLAGS += -DTUI_ENABLE_DISPLAY=1
ARMCC_COMPILATION_FLAGS += -DTUI_ENABLE_TOUCH=1
ARMCC_COMPILATION_FLAGS += -DTUI_LOCK_I2C=1
ARMCC_COMPILATION_FLAGS += -DTUI_ENABLE_MPU=0
#ARMCC_COMPILATION_FLAGS += -Werror
ARMCC_COMPILATION_FLAGS += -Wno-error
# use generic make file
PLATFORM ?= ARM_V7A_STD
DRIVER_DIR ?= Locals/Code
DRLIB_DIR := $(DRIVER_DIR)/Api
DRSDK_DIR_SRC ?= $(DRSDK_DIR)
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(DRSDK_DIR)/driver.mk
