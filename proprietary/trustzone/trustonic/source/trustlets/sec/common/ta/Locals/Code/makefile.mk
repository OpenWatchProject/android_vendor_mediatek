################################################################################
#
# Sec Trustlet (Sec TA)
#
################################################################################


# output binary name without path or extension
OUTPUT_NAME := tlsec

GP_ENTRYPOINTS := Y

#-------------------------------------------------------------------------------
# MobiConvert parameters, see manual for details
#-------------------------------------------------------------------------------

TRUSTLET_UUID := 05110000000000000000000000000000
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
HEAP_SIZE_MAX := 65536

#-------------------------------------------------------------------------------
# Library to include
#-------------------------------------------------------------------------------

CUSTOMER_DRIVER_LIBS += $(MSEE_FWK_TA_OUT_DIR)/$(TEE_MODE)/msee_fwk_ta.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/drsec_api_intermediates/drsec_api.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/tasec_core_intermediates/tasec_core.lib
#OUTPUT_TO_DRIVER_FOLDER := yes

#-------------------------------------------------------------------------------
# use generic make file
TRUSTLET_DIR ?= src
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
ifeq (311c, $(TRUSTONIC_TEE_VERSION))
  include $(TLSDK_DIR)/trustlet_400b.mk
else
  include $(TLSDK_DIR)/trustlet.mk
endif
