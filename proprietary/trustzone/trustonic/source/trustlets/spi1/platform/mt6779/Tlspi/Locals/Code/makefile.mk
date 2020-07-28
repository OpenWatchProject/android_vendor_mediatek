################################################################################
#
# <t-sdk SPI Trustlet
#
################################################################################


# output binary name without path or extension
OUTPUT_NAME := tlspi


#-------------------------------------------------------------------------------
# MobiConvert parameters, see manual for details
#-------------------------------------------------------------------------------

TRUSTLET_UUID := 09140000000000000000000000000000
TRUSTLET_MEMTYPE := 2
TRUSTLET_NO_OF_THREADS := 1
TRUSTLET_SERVICE_TYPE := 3
TRUSTLET_KEYFILE := Locals/Build/pairVendorTltSig.pem
TRUSTLET_FLAGS := 0
TRUSTLET_INSTANCES := 10


#-------------------------------------------------------------------------------
# For 302A and later version
#-------------------------------------------------------------------------------
TBASE_API_LEVEL := 5
HEAP_SIZE_INIT := 4096
HEAP_SIZE_MAX := 65536

#-------------------------------------------------------------------------------
# Files and include paths - Add your files here
#-------------------------------------------------------------------------------

### Add include path here
INCLUDE_DIRS += \
    Locals/Code/public \
    $(COMP_PATH_Secmem_Export) \
    $(COMP_PATH_drsec)/public \
    $(DRSPI_DIR)/Locals/Code/public \

### Add source code files for C++ compiler here
SRC_CPP += \
    Locals/Code/tlspi.c

### Add source code files for C compiler here
SRC_C += # nothing

### Add source code files for assembler here
SRC_S += # nothing

ARMCC_COMPILATION_FLAGS += -Werror

ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)
CUSTOMER_DRIVER_LIBS += $(DRSPI_OUT_DIR)/$(TEE_MODE)/drspi.lib
CUSTOMER_DRIVER_LIBS += $(MSEE_FWK_TA_OUT_DIR)/$(TEE_MODE)/msee_fwk_ta.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/drsec_api_intermediates/drsec_api.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/drutils_api_intermediates/drutils_api.lib
else
CUSTOMER_DRIVER_LIBS += $(DRSPI_OUT_DIR)/$(TEE_MODE)/drspi.lib
CUSTOMER_DRIVER_LIBS += $(DRUTILS_OUT_DIR)/$(TEE_MODE)/drutils.lib
CUSTOMER_DRIVER_LIBS += $(DRSEC_OUT_DIR)/$(TEE_MODE)/drsec.lib
endif # !MTK_TEE_GP_SUPPORT

#-------------------------------------------------------------------------------
# use generic make file
TRUSTLET_DIR := Locals/Code
TLSDK_DIR_SRC := $(TLSDK_DIR)
include $(TLSDK_DIR)/trustlet.mk

