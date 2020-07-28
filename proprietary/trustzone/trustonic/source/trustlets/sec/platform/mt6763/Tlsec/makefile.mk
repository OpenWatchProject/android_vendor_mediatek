################################################################################
#
# <t-sdk Sec Trustlet
#
################################################################################


# output binary name without path or extension
OUTPUT_NAME := tlsec


#-------------------------------------------------------------------------------
# MobiConvert parameters, see manual for details
#-------------------------------------------------------------------------------

TRUSTLET_UUID := 05110000000000000000000000000000
TRUSTLET_MEMTYPE := 2
TRUSTLET_NO_OF_THREADS := 1
TRUSTLET_SERVICE_TYPE := 3
TRUSTLET_KEYFILE := pairVendorTltSig.pem
TRUSTLET_FLAGS := 0
TRUSTLET_INSTANCES := 10

#-------------------------------------------------------------------------------
# For 302A and later version
#-------------------------------------------------------------------------------
TBASE_API_LEVEL := 5
HEAP_SIZE_INIT := 4096
HEAP_SIZE_MAX := 65536

#-------------------------------------------------------------------------------
# use generic make file
TRUSTLET_DIR ?= Locals/Code
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(TLSDK_DIR)/trustlet_release.mk
