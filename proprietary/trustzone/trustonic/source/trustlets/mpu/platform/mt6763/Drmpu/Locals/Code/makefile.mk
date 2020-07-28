################################################################################
#
# EMI MPU library for secure memory / TUI drivers
#
################################################################################

# output binary name without path or extension
OUTPUT_NAME := mpu_api
BUILD_DRIVER_LIBRARY_ONLY := YES

DRIVER_UUID := b4f913afd60a4599ac79827890cdb3db
#DRIVER_MEMTYPE := 2 # 0: iram preferred; 1: iram; 2: eram
#DRIVER_NO_OF_THREADS := 4 # min =1; max =8
#DRIVER_SERVICE_TYPE := 1 # 1: driver; 2: service provider trustlet; 3: system trustlet
#DRIVER_KEYFILE := Locals/Build/pairVendorTltSig.pem
#DRIVER_FLAGS:= 0                # 0: no flag; 1: permanent; 2: service has no WSM control interface; 3: both (permanent and service has not WSM control interface)
#DRIVER_VENDOR_ID :=0 # Trustonic
#DRIVER_NUMBER := 0x0727         # Drutils
#DRIVER_ID := $$(($(DRIVER_VENDOR_ID)<<16|$(DRIVER_NUMBER)))
#DRIVER_INTERFACE_VERSION_MAJOR=1
#DRIVER_INTERFACE_VERSION_MINOR=0
#DRIVER_INTERFACE_VERSION := $(DRIVER_INTERFACE_VERSION_MAJOR).$(DRIVER_INTERFACE_VERSION_MINOR)

#-------------------------------------------------------------------------------
# For 302A and later version
#-------------------------------------------------------------------------------
TBASE_API_LEVEL := 5
#HEAP_SIZE_INIT := 4096
#HEAP_SIZE_MAX := 65536

#-------------------------------------------------------------------------------
# DCI handler and trustlet APIs are enabled as default
#-------------------------------------------------------------------------------
#USE_DCI_HANDLER := YES
#USE_TL_API := YES
#USE_MT_DRV_FWK := YES

#-------------------------------------------------------------------------------
# Files and include paths - Add your files here
#-------------------------------------------------------------------------------

### Add include path here
INCLUDE_DIRS += \
    Locals/Code/inc \
    Locals/Code/public

### Add library source code files for C compiler here
SRC_CORE_LIB_C += \
    Locals/Code/DrApiEmiMpu.c \
    Locals/Code/DrApiPlat.c \

### Add source code files for C++ compiler here
SRC_CPP += # nothing

### Add source code files for assembler here
SRC_ASM += # nothing

#ARMCC_COMPILATION_FLAGS += -DDRIVER_ID=$(DRIVER_ID)
#ARMCC_COMPILATION_FLAGS += -DDRIVER_VERSION_MAJOR=$(DRIVER_INTERFACE_VERSION_MAJOR)
#ARMCC_COMPILATION_FLAGS += -DDRIVER_VERSION_MINOR=$(DRIVER_INTERFACE_VERSION_MINOR)
ARMCC_COMPILATION_FLAGS += -fshort-wchar
ARMCC_COMPILATION_FLAGS += -Werror -Wall -Wextra

# Use -fno-short-enums for message_t
# IPC ipchLoop(): Invalid message 0x0000cc03 from sender=0x00020005
#ARMCC_SHORT_ENUMS := YES

#-------------------------------------------------------------------------------
# use generic make file
DRIVER_DIR ?= .
DRLIB_DIR := $(DRIVER_DIR)/Locals/Code
DRSDK_DIR_SRC ?= $(DRSDK_DIR)
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(DRSDK_DIR)/driver.mk
