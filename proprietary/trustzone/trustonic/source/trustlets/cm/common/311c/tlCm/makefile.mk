################################################################################
#
# Trustonic Content Management TA
#
################################################################################


# output binary name without path or extension
OUTPUT_NAME := tlCm


#-------------------------------------------------------------------------------
# MobiConvert parameters, see manual for details
#-------------------------------------------------------------------------------

TRUSTLET_UUID := 07010000000000000000000000000000
TRUSTLET_MEMTYPE := 2
TRUSTLET_NO_OF_THREADS := 1
TRUSTLET_SERVICE_TYPE := 3
TRUSTLET_KEYFILE := pairVendorTltSig.pem
TRUSTLET_MOBICONFIG_KEY := MobiConfig_GuD_KPH_public_key.pem
TRUSTLET_MOBICONFIG_KID := 1
TRUSTLET_MOBICONFIG_USE := true
TRUSTLET_FLAGS := 0
TRUSTLET_INSTANCES := 1
#-------------------------------------------------------------------------------
# for t-base 302A
#-------------------------------------------------------------------------------
TBASE_API_LEVEL := 3

#-------------------------------------------------------------------------------
# use generic make file
TRUSTLET_DIR ?= Locals/Code
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(TLSDK_DIR)/trustlet_release.mk
