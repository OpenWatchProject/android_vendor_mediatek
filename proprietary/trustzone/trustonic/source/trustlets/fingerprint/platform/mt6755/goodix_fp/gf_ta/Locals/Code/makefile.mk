################################################################################
#
# FingerPrint Trusted Application
#
################################################################################

# output binary name without path or extension
OUTPUT_NAME := gf_ta
GP_ENTRYPOINTS := Y

TEE := trustonic
$(info "MODE:-------"${MODE})
#-------------------------------------------------------------------------------
# t-base-convert parameters, see manual for details
#-------------------------------------------------------------------------------
TRUSTLET_UUID := 05060000000000000000000000000000
TRUSTLET_MEMTYPE := 2
TRUSTLET_NO_OF_THREADS := 1
TRUSTLET_SERVICE_TYPE := 3
TRUSTLET_KEYFILE := Locals/Build/pairVendorTltSig.pem

ifeq ($(MODE), Release)
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
# Goodix Fingerprint chip type:
# GF316M/GF318M/GF3118M/GF518M/GF5118M/GF516M/GF816M/GF3208/GF5216. etc
############################################################################################
GF_CHIP_TYPE := GF318M

ARMCC_COMPILATION_FLAGS += -D${GF_CHIP_TYPE} \
                           -Werror


ifeq ($(ANDROID_VERSION),android5.1)
ARMCC_COMPILATION_FLAGS +=-D__ANDROID_51
endif

# GF_LOG_LEVEL is 3, will output debug, info, error log
# GF_LOG_LEVEL is 2, will output info, error log
# GF_LOG_LEVEL is 1, only output error log
ifeq (${TEE_MODE}, Debug)
ARMCC_COMPILATION_FLAGS += -DGF_LOG_LEVEL=3
else
ARMCC_COMPILATION_FLAGS += -DGF_LOG_LEVEL=2
endif

#-------------------------------------------------------------------------------
# Files and include paths - Add your files here
#-------------------------------------------------------------------------------

### Add include path here
INCLUDE_DIRS +=  \
    Locals/Code \
    Locals/Code/gf_lib \
    Locals/Code/fw_cfg \
    ../public \
    ../../../../../spi/platform/$(ARCH_MTK_PLATFORM)/Drspi/Locals/Code/public \
    ../../../../../../external/mobicore/common/302c/MobiCoreDriverLib \
    ../gf_dr/Locals/Code/public

INCLUDE_DIRS +=  \
    Locals/Code/tee/$(TEE) \
    Locals/Code/tee/include

### Add source code files for C++ compiler here
SRC_CPP += \
    Locals/Code/tee/$(TEE)/gf_ta_entry.c \
    Locals/Code/tee/$(TEE)/gf_tee_internal_api.c \
    Locals/Code/tee/$(TEE)/gf_tee_storage.c \
    Locals/Code/tee/$(TEE)/gf_tee_call_securedriver.c \
    Locals/Code/tee/$(TEE)/gf_tee_memory.c \
    Locals/Code/tee/$(TEE)/gf_tee_time.c \
    Locals/Code/tee/$(TEE)/gf_spi.c \
    Locals/Code/gf_config.c


### Add source code files for C compiler here
SRC_C += # nothing

### Add source code files for assembler here
SRC_S += # nothing

BUILD_MODE := $(shell echo ${TEE_MODE} | tr '[A-Z]' '[a-z]')
ifeq ($(GF_CHIP_TYPE),$(filter $(GF_CHIP_TYPE), GF318M GF3118M GF518M GF5118M GF318 GF3118 GF518 GF5118))
CUSTOMER_DRIVER_LIBS += Locals/Code/gf_lib/$(BUILD_MODE)/oswego_96/gf_ta.lib
else ifeq ($(GF_CHIP_TYPE),$(filter $(GF_CHIP_TYPE), GF316M GF516M GF516 GF816M))
CUSTOMER_DRIVER_LIBS += Locals/Code/gf_lib/$(BUILD_MODE)/oswego_118/gf_ta.lib
else ifeq ($(GF_CHIP_TYPE), $(filter $(GF_CHIP_TYPE), GF3208))
CUSTOMER_DRIVER_LIBS += Locals/Code/gf_lib/$(BUILD_MODE)/milan_108/gf_ta.lib
endif
CUSTOMER_DRIVER_LIBS += $(GF_DR_OUT_DIR)/$(TEE_MODE)/gf_dr.lib

CUSTOMER_DRIVER_LIBS += $(DRSPI_OUT_DIR)/$(TEE_MODE)/drspi.lib
CUSTOMER_DRIVER_LIBS += $(DRSEC_OUT_DIR)/$(TEE_MODE)/drsec.lib
CUSTOMER_DRIVER_LIBS += $(DRUTILS_OUT_DIR)/$(TEE_MODE)/drutils.lib
CUSTOMER_DRIVER_LIBS += Locals/Code/gf_lib/get_hmac_key.lib

#-------------------------------------------------------------------------------
# use generic make file
TRUSTLET_DIR ?= Locals/Code
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(TLSDK_DIR)/trustlet.mk
