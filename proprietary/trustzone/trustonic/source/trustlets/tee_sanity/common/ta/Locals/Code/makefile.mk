################################################################################
#
# Sec Trustlet
#
################################################################################


# output binary name without path or extension
OUTPUT_NAME := tltee_sanity
GP_ENTRYPOINTS := Y

#-------------------------------------------------------------------------------
# MobiConvert parameters, see manual for details
#-------------------------------------------------------------------------------

TRUSTLET_UUID := 05150000000000000000000000000000
TRUSTLET_MEMTYPE := 2
TRUSTLET_NO_OF_THREADS := 1
TRUSTLET_SERVICE_TYPE := 3
TRUSTLET_KEYFILE := Locals/Build/pairVendorTltSig.pem
TRUSTLET_FLAGS := 0
TRUSTLET_INSTANCES := 10

#-------------------------------------------------------------------------------
# For t-base 302A
#-------------------------------------------------------------------------------
TBASE_API_LEVEL := 5
HEAP_SIZE_INIT := 4096
HEAP_SIZE_MAX := 1048576 # //1MB
#HEAP_SIZE_MAX := 65536 # //64KB

#-------------------------------------------------------------------------------
# Library to include
#-------------------------------------------------------------------------------

CUSTOMER_DRIVER_LIBS += $(MSEE_FWK_TA_OUT_DIR)/$(TEE_MODE)/msee_fwk_ta.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/drtee_sanity_api_intermediates/drtee_sanity_api.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/tatee_sanity_core_intermediates/tatee_sanity_core.lib

#-------------------------------------------------------------------------------
# use generic make file
TRUSTLET_DIR ?= src
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(TLSDK_DIR)/trustlet.mk
