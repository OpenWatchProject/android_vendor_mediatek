################################################################################
#
# GF Alipay Trusted Application
#
################################################################################

# output binary name without path or extension
OUTPUT_NAME := gf_alipay_ta
GP_ENTRYPOINTS := Y

TEE := trustonic
$(info "MODE:-------"${MODE})
#-------------------------------------------------------------------------------
# t-base-convert parameters, see manual for details
#-------------------------------------------------------------------------------

ifeq ($(TEE_MODE), Release)
    ARMCC_COMPILATION_FLAGS += -DGF_LOG_LEVEL=2
else
    ARMCC_COMPILATION_FLAGS += -DGF_LOG_LEVEL=0
endif

ARMCC_COMPILATION_FLAGS += -DCFG_TRACE_LEVEL=6
ARMCC_COMPILATION_FLAGS += -D__TEE_TRUSTONIC

TRUSTLET_UUID := 05080000000000000000000000000000
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
    Locals/Code/Api/$(TEE)

### Add source code files for C++ compiler here
SRC_CPP += # nothing

### Add source code files for C compiler here
SRC_C +=  \
    Locals/Code/Api/$(TEE)/main.c \
    Locals/Code/oem.c

### Add source code files for assembler here
SRC_S += # nothing

ALIPAY_TEST_VERSION := false
ifeq ($(ALIPAY_TEST_VERSION), true)
CUSTOMER_DRIVER_LIBS += ./Locals/Code/lib/Test/alipay.lib
else
CUSTOMER_DRIVER_LIBS += ./Locals/Code/lib/$(MODE)/alipay.lib
endif
CUSTOMER_DRIVER_LIBS += $(GF_DR_OUT_DIR)/$(TEE_MODE)/gf_dr.lib

#-------------------------------------------------------------------------------
# use generic make file
TRUSTLET_DIR ?= Locals/Code
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(TLSDK_DIR)/trustlet.mk
