#
# Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
# All Rights Reserved.
#
################################################################################
#
# FingerPrint Trusted Application
#
################################################################################

# output binary name without path or extension
OUTPUT_NAME := gf_ta
GP_ENTRYPOINTS := Y

TEE := trustonic

ifeq ($(TARGET_BUILD_VARIANT), eng)
#TARGET_MODE := debug
#for kernel log too much
TARGET_MODE := release
else ifeq ($(TARGET_BUILD_VARIANT), userdebug)
#TARGET_MODE := userdebug
#for kernel log too much
TARGET_MODE := release
else
TARGET_MODE := release
endif
#-------------------------------------------------------------------------------
# t-base-convert parameters, see manual for details
#-------------------------------------------------------------------------------
TRUSTLET_UUID := 05060000000000000000000000000000
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

TBASE_API_LEVEL := 5
ifeq ($(TBASE_API_LEVEL), 5)
HEAP_SIZE_INIT := 5242880
HEAP_SIZE_MAX := 5242880
endif

############################################################################################
# define ARMCC_COMPILATION_FLAGS
############################################################################################
-include Locals/Code/config.mk

ARMCC_COMPILATION_FLAGS += \
    -Werror \
    -g0

ARMCC_COMPILATION_FLAGS += $(TA_CHIP_FLAGS)

#-------------------------------------------------------------------------------
# Files and include paths - Add your files here
#-------------------------------------------------------------------------------
#TA_DIRS = $(shell find Locals/Code/gf_lib/$(GF_CHIP_SERIES) -type d)
#TA_DIRS += $(shell find Locals/Code/platform -type d)
#TA_DIRS = $(shell find gf_lib/$(GF_CHIP_SERIES) -type d)
#TA_DIRS += $(shell find platform -type d)
ifneq ($(wildcard Locals/Code),)
TA_DIRS = $(shell find Locals/Code/gf_lib/$(GF_CHIP_SERIES) -type d)
TA_DIRS += $(shell find Locals/Code/platform -type d)
endif
TA_DIRS += Locals/Code/gf_lib/include \
    Locals/Code/gf_lib/public \
    Locals/Code/gf_lib/common \
    Locals/Code/gf_lib/algo/common \
    Locals/Code/gf_lib/algo/$(GF_CHIP_SERIES) \
    Locals/Code/$(GF_CHIP_SERIES) \
    Locals/Code/public

### Add include path here
INCLUDE_DIRS += \
    ${TA_DIRS} \
    ../public \
    ../include \
    ../gf_dr/Locals/Code/public

### Add AndroidM include paths
INCLUDE_DIRS += \
    ../../../../../spi/platform/$(ARCH_MTK_PLATFORM)/Drspi/Locals/Code/public \
    ../../../../../../external/mobicore/common/302d/MobiCoreDriverLib

### Add AndroidL include paths
#INCLUDE_DIRS += \
    ../../spi/Drspi/Locals/Code/public \
    ../../../../../external/mobicore/common/302d/MobiCoreDriverLib

### Add source code files for C++ compiler here
SRC_CPP := \
    $(foreach dir, $(TA_DIRS), $(wildcard $(dir)/*.c)) \
    ../public/gf_error.c \
    ../public/gf_type_define.c \
    ../include/gf_common.c

INCLUDE_DIRS += $(SRC_CPP_DIRS)

ifeq ($(TARGET_MODE), release)
SRC_CPP := $(filter-out Locals/Code/gf_lib/common/gf_ta_dump.c,$(SRC_CPP))
else
SRC_CPP := $(filter-out Locals/Code/gf_lib/common/gf_ta_dump_stub.c,$(SRC_CPP))
endif


### Add source code files for C compiler here
SRC_C += # nothing

### Add source code files for assembler here
SRC_S += # nothing

CUSTOMER_DRIVER_LIBS += $(GF_DR_OUT_DIR)/$(TEE_MODE)/gf_dr.lib

ifeq ($(CPU_TYPE), samsung)
CUSTOMER_DRIVER_LIBS += ../prebuilts/tee/${TEE}/${TEE_VERSION}/trustlets/spi/Drspi/release/secxxxdrv.lib
else
CUSTOMER_DRIVER_LIBS += $(DRSPI_OUT_DIR)/$(TEE_MODE)/drspi.lib
ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)
    CUSTOMER_DRIVER_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/drsec_api_intermediates/drsec_api.lib
    CUSTOMER_DRIVER_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/drutils_api_intermediates/drutils_api.lib
    CUSTOMER_DRIVER_LIBS += $(MSEE_FWK_TA_OUT_DIR)/$(TEE_MODE)/msee_fwk_ta.lib
else
CUSTOMER_DRIVER_LIBS += $(DRSEC_OUT_DIR)/$(TEE_MODE)/drsec.lib

HAVE_DRUTILS_LIB_FILE := $(shell test ! -f $(DRUTILS_OUT_DIR)/$(TEE_MODE)/drutils.lib && echo no)
ifeq ($(HAVE_DRUTILS_LIB_FILE), no)
    # There is no Debug drutils.lib on Android L
    CUSTOMER_DRIVER_LIBS += $(DRUTILS_OUT_DIR)/Release/drutils.lib
else
    CUSTOMER_DRIVER_LIBS += $(DRUTILS_OUT_DIR)/$(TEE_MODE)/drutils.lib
endif

endif


endif

CUSTOMER_DRIVER_LIBS += Locals/Code/gf_lib/$(TARGET_MODE)/$(GF_CHIP_SERIES)/$(TEE)/libgf_ta_algo.a
CUSTOMER_DRIVER_LIBS += Locals/Code/platform/trustonic/libs/get_key.lib

#-------------------------------------------------------------------------------
# use generic make file
TRUSTLET_DIR ?= Locals/Code
TLSDK_DIR_SRC ?= $(TLSDK_DIR)

ifneq ($(BUILD_WITH_NDK), true)
include $(TLSDK_DIR)/trustlet.mk
endif
