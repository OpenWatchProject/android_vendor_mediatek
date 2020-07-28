#
# Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
# All Rights Reserved.
#

################################################################################
#
# GF Alipay Trusted Application
#
################################################################################

# output binary name without path or extension
OUTPUT_NAME := gf_ifaa_ta
GP_ENTRYPOINTS := Y

TEE := trustonic
#-------------------------------------------------------------------------------
# t-base-convert parameters, see manual for details
#-------------------------------------------------------------------------------

# GF_LOG_LEVEL is 4, will output verbose, debug, info, error log
# GF_LOG_LEVEL is 3, will output debug, info, error log
# GF_LOG_LEVEL is 2, will output info, error log
# GF_LOG_LEVEL is 1, only output error log
ifeq ($(TARGET_BUILD_VARIANT), eng)
ARMCC_COMPILATION_FLAGS += -DGF_LOG_LEVEL=4
else ifeq ($(TARGET_BUILD_VARIANT), userdebug)
ARMCC_COMPILATION_FLAGS += -DGF_LOG_LEVEL=3
else
ARMCC_COMPILATION_FLAGS += -DGF_LOG_LEVEL=1
endif

ARMCC_COMPILATION_FLAGS += -DCFG_TRACE_LEVEL=6
ARMCC_COMPILATION_FLAGS += -D__TEE_TRUSTONIC

TRUSTLET_UUID := 05090000000000000000000000000000
TRUSTLET_MEMTYPE := 2
TRUSTLET_NO_OF_THREADS := 1
TRUSTLET_SERVICE_TYPE := 3
TRUSTLET_KEYFILE := Locals/Build/pairVendorTltSig.pem

ifeq ($(TEE_MODE), Release)
    TRUSTLET_FLAGS = 0
else
    TRUSTLET_FLAGS = 4
endif
TRUSTLET_INSTANCES := 1

#-------------------------------------------------------------------------------

TBASE_API_LEVEL := 5
ifeq ($(TBASE_API_LEVEL), 5)
    HEAP_SIZE_INIT := 4194304 # 4MB
    HEAP_SIZE_MAX := 4194304
endif

#-------------------------------------------------------------------------------
# Files and include paths - Add your files here
#-------------------------------------------------------------------------------

### Add include path here
INCLUDE_DIRS +=  \
    Locals/Code \
    Locals/Code/$(TEE) \
    ../public

### Add source code files for C++ compiler here
SRC_CPP += # nothing

### Add source code files for C compiler here
SRC_C +=  \
    Locals/Code/$(TEE)/main.c

### Add source code files for assembler here
SRC_S += # nothing

CUSTOMER_DRIVER_LIBS += $(GF_DR_OUT_DIR)/$(TEE_MODE)/gf_dr.lib

#-------------------------------------------------------------------------------
# use generic make file
TRUSTLET_DIR ?= Locals/Code
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(TLSDK_DIR)/trustlet.mk
