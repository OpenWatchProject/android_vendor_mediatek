#
# Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
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
HEAP_SIZE_INIT := 8388608
HEAP_SIZE_MAX := 8388608
endif

############################################################################################
# Goodix Fingerprint chip type:
# GF316M/GF318M/GF3118M/GF518M/GF5118M/GF516M/GF816M/GF3208/GF5206/GF5216/GF5208. etc
############################################################################################
GF_CHIP_TYPE := GF5216
CPU_TYPE := mtk
ARMCC_COMPILATION_FLAGS += \
    -Werror \
    -g0 \
    -D${GF_CHIP_TYPE} \
    -DGF_TA_VERSION=\"188750cb3fb98c6991b9064876bc440a44fea4f1_2018.07.24_18:37:23\" \
    -DGF_AUTHENTICATOR_VERSION_CODE=2018072418 \
    -DGF_CHIP_TYPE=\"${GF_CHIP_TYPE}\" \
    -DGF_TARGET_MODE=\"${TEE_MODE}\" \
    -DGF_TEE=\"$(TEE)\" \
    -DGF_PACKAGE_VERSION_CODE=\"118\" \
    -DGF_PACKAGE_VERSION_NAME=\"V1.1.8\" \
    -DGF_GIT_BRANCH=\"milan_for_mtk_r4\" \
    -DGF_COMMIT_ID=\"188750cb3fb98c6991b9064876bc440a44fea4f1\" \
    -DGF_BUILD_TIME=\"2018.07.24_18:37:23\" \

# GF_LOG_LEVEL is 3, will output debug, info, error log
# GF_LOG_LEVEL is 2, will output info, error log
# GF_LOG_LEVEL is 1, only output error log
ifeq (${TEE_MODE}, Debug)
#ARMCC_COMPILATION_FLAGS += -DGF_LOG_LEVEL=3
#for kernel log too much
ARMCC_COMPILATION_FLAGS += -DGF_LOG_LEVEL=1
else
#ARMCC_COMPILATION_FLAGS += -DGF_LOG_LEVEL=2
#for kernel log too much
ARMCC_COMPILATION_FLAGS += -DGF_LOG_LEVEL=1
endif

GF_OSWEGO_USE_MILAN_ALGO := false

BUILD_MODE := $(shell echo ${TEE_MODE} | tr '[A-Z]' '[a-z]')
ifeq ($(GF_CHIP_TYPE), $(filter $(GF_CHIP_TYPE), GF5206 GF5216 GF5208))
    #CUSTOMER_DRIVER_LIBS += Locals/Code/gf_lib/$(BUILD_MODE)/milan_a_series/gf_ta_algo.lib
    #for kernel log too much
    CUSTOMER_DRIVER_LIBS += Locals/Code/gf_lib/release/milan_a_series/gf_ta_algo.lib
    GF_CHIP_SERIES_NAME=milan_a_series

    ARMCC_COMPILATION_FLAGS += -DGF_RAWDATA_LEN=19360 -DGF_CHIP_SERIES_MILAN_A
endif

#-------------------------------------------------------------------------------
# Files and include paths - Add your files here
#-------------------------------------------------------------------------------

### Add include path here
INCLUDE_DIRS +=  \
    Locals/Code \
    Locals/Code/gf_lib \
    Locals/Code/fw_cfg \
    Locals/Code/gf_lib/algo \
    Locals/Code/gf_lib/cpl/inc \
    Locals/Code/gf_lib/spi \
    Locals/Code/gf_lib/base \
    Locals/Code/platform/$(TEE) \
    Locals/Code/platform/include \
    Locals/Code/gf_lib/spi/$(GF_CHIP_SERIES_NAME) \
    ../public \
    ../gf_dr/Locals/Code/public \
    ../../../../../../../source/trustlets/spi/platform/mt6761/Drspi/Locals/Code/public \
    ../../../../../../external/mobicore/common/400b/ClientLib/src
   

    

### Add source code files for C++ compiler here
SRC_CPP_DIRS := $(shell find Locals/Code/ -maxdepth 6 -type d | sed -e '/platform/d;/samsung/d;/oswego/d;/milan_f_series/d;')
SRC_CPP_DIRS += Locals/Code/platform/$(TEE) \
                Locals/Code/platform/$(TEE)/mtk

SRC_CPP := $(foreach dir, $(SRC_CPP_DIRS), $(wildcard $(dir)/*.c))
$(info ================th==========$(SRC_CPP_DIRS) \n==========\n$(SRC_CPP))
INCLUDE_DIRS += $(SRC_CPP_DIRS)

### Add source code files for C compiler here
SRC_C += # nothing

### Add source code files for assembler here
SRC_S += # nothing

GF_MEMORY_DEBUG_ENABLE := false

ifeq ($(GF_MEMORY_DEBUG_ENABLE), true)
    INCLUDE_DIRS += Locals/Code/gf_lib/memory/inc
    SRC_CPP += Locals/Code/gf_lib/memory/src/mem_manager.c
    ARMCC_COMPILATION_FLAGS += -DGF_MEMORY_DEBUG_ENABLE
endif

GF_DR_OUT_DIR := ../out/gf_dr
CUSTOMER_DRIVER_LIBS += $(GF_DR_OUT_DIR)/$(TEE_MODE)/gf_dr.lib

CUSTOMER_DRIVER_LIBS += $(DRSPI0_OUT_DIR)/$(TEE_MODE)/drspi0.lib
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


CUSTOMER_DRIVER_LIBS += Locals/Code/gf_lib/algo/hmac/lib/get_hmac_key.lib

#-------------------------------------------------------------------------------
# use generic make file
TRUSTLET_DIR ?= Locals/Code
TLSDK_DIR_SRC ?= $(TLSDK_DIR)

ifneq ($(BUILD_WITH_NDK), true)
	include $(TLSDK_DIR)/trustlet.mk
endif
