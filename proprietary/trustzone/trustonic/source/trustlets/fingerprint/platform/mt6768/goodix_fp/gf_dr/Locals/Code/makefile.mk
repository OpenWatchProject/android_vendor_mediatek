#
# Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
# All Rights Reserved.
#


# output binary name without path or extension
OUTPUT_NAME := gf_dr


#-------------------------------------------------------------------------------
# MobiConvert parameters, see manual for details
#-------------------------------------------------------------------------------
DRIVER_UUID := 05070000000000000000000000000000
DRIVER_MEMTYPE := 2 # 0: iram preferred; 1: iram; 2: eram
DRIVER_NO_OF_THREADS := 3 # min =1; max =8
DRIVER_SERVICE_TYPE := 1 # 1: driver; 2: service provider trustlet; 3: system trustlet
DRIVER_KEYFILE := Locals/Build/pairVendorTltSig.pem
DRIVER_FLAGS:= 0 # 0: no flag; 1: permanent; 2: service has no WSM control interface; 3: both (permanent and service has not WSM control interface)
DRIVER_VENDOR_ID :=0 # Trustonic
DRIVER_NUMBER := 0x8001 # gf_dr
DRIVER_ID := $$(($(DRIVER_VENDOR_ID)<<16|$(DRIVER_NUMBER)))
DRIVER_INTERFACE_VERSION_MAJOR=1
DRIVER_INTERFACE_VERSION_MINOR=0
DRIVER_INTERFACE_VERSION := $(DRIVER_INTERFACE_VERSION_MAJOR).$(DRIVER_INTERFACE_VERSION_MINOR)

TBASE_API_LEVEL := 5
HEAP_SIZE_INIT := 1048576
HEAP_SIZE_MAX := 1048576

# GF_LOG_LEVEL is 3, will output debug, info, error log
# GF_LOG_LEVEL is 2, will output info, error log
# GF_LOG_LEVEL is 1, only output error log
ifeq ($(TARGET_MODE), debug)
ARMCC_COMPILATION_FLAGS += -DGF_LOG_LEVEL=3
else
ARMCC_COMPILATION_FLAGS += -DGF_LOG_LEVEL=1
endif

#-------------------------------------------------------------------------------
# Files and include paths - Add your files here
#-------------------------------------------------------------------------------

### Add include path here
INCLUDE_DIRS += \
    Locals/Code \
    Locals/Code/public \
    Locals/Code/tee \
    ../public

### Add library source code files for C compiler here
SRC_LIB_C += \
    Locals/Code/gf_dr.c \
    Locals/Code/gf_alipay_api.c \
    Locals/Code/gf_wechat_api.c \
    Locals/Code/gf_ifaa_api.c

### Add source code files for C++ compiler here
SRC_CPP += \
    Locals/Code/tee/drMain.c \
    Locals/Code/tee/drIpch.c \
    Locals/Code/tee/drExch.c \
    Locals/Code/gf_dr_ipch.c

### Add source code files for C compiler here
SRC_C += #nothing

### Add source code files for assembler here
SRC_ASM += # nothing

ARMCC_COMPILATION_FLAGS += -DDRIVER_ID=$(DRIVER_ID)
ARMCC_COMPILATION_FLAGS += -DDRIVER_VERSION_MAJOR=$(DRIVER_INTERFACE_VERSION_MAJOR)
ARMCC_COMPILATION_FLAGS += -DDRIVER_VERSION_MINOR=$(DRIVER_INTERFACE_VERSION_MINOR)

#-------------------------------------------------------------------------------
# use generic make file
DRIVER_DIR ?= Locals/Code
DRLIB_DIR := $(DRIVER_DIR)
DRSDK_DIR_SRC ?= $(DRSDK_DIR)
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(DRSDK_DIR)/driver.mk

