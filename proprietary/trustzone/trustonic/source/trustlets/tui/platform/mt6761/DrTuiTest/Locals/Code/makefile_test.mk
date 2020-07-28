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

TBASE_API_LEVEL:=3


### Add include path here
INCLUDE_DIRS += \
    Locals/Code/inc \
    Locals/Code/public \
    Locals/Code/devices/$(DEVICE_PATH) \
    Locals/Code/platforms \

# For EMI MPU APIs
LOCAL_TRUSTZONE_PROTECT_BSP_PATH=../../../../../../../../../../../vendor/mediatek/proprietary/trustzone/trustonic/source/trustlets
INCLUDE_DIRS += $(LOCAL_TRUSTZONE_PROTECT_BSP_PATH)/mpu/platform/${ARCH_MTK_PLATFORM}/Drmpu/Locals/Code/public \
INCLUDE_DIRS += $(LOCAL_TRUSTZONE_PROTECT_BSP_PATH)/mpu/common/Drmpu/Locals/Code/public \

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
#DRTUICORE_LIB := Out/Bin/$(MODE)/drTuiCore.a
#EXTRA_LIBS := $(DRTUICORE_LIB)

SRC_C += \
    Locals/Code/testMain.c \
    Locals/Code/testDrSmgmt.c \
    Locals/Code/testDrUtils.c

# ************************************************************************************************

# output binary name without path or extension
OUTPUT_NAME := drTuiTest

include Locals/Code/driverTuiTest.mk

ARMCC_COMPILATION_FLAGS += -DDRIVER_ID=$(DRIVER_ID)
ARMCC_COMPILATION_FLAGS += -DDRIVER_VERSION_MAJOR=$(DRIVER_INTERFACE_VERSION_MAJOR)
ARMCC_COMPILATION_FLAGS += -DDRIVER_VERSION_MINOR=$(DRIVER_INTERFACE_VERSION_MINOR)

# use generic make file
PLATFORM ?= ARM_V7A_STD
DRIVER_DIR ?= Locals/Code
DRLIB_DIR := $(DRIVER_DIR)/api
DRSDK_DIR_SRC ?= $(DRSDK_DIR)
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(DRSDK_DIR)/driver.mk
