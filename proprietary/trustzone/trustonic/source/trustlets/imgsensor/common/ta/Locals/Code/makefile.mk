################################################################################
#
# <t-sdk ISP Trustlet
#
################################################################################

ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)

# output binary name without path or extension
OUTPUT_NAME := tlimgsensor
GP_ENTRYPOINTS := Y

#-------------------------------------------------------------------------------
# MobiConvert parameters, see manual for details
#-------------------------------------------------------------------------------

TRUSTLET_UUID := 7b66512021214487ba710a51d7ea78fe
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
# Library to include
#-------------------------------------------------------------------------------
CUSTOMER_DRIVER_LIBS += $(MSEE_FWK_TA_OUT_DIR)/$(TEE_MODE)/msee_fwk_ta.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/ImgsensorTA_intermediates/ImgsensorTA.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/drutils_core_intermediates/drutils_core.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/drutils_api_intermediates/drutils_api.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/drsec_api_intermediates/drsec_api.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/drisp_api_intermediates/drisp_api.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/libimgsensor_sec_custom_intermediates/libimgsensor_sec_custom.lib

$(info include i2c? TEE_MACH_TYPE = $(strip $(TEE_MACH_TYPE)))
ifeq ($(TEE_MACH_TYPE), mt6771)
CUSTOMER_DRIVER_LIBS += $(DRI2C_OUT_DIR)/$(TEE_MODE)/drI2C.lib
endif

#-------------------------------------------------------------------------------
# use generic make file
TRUSTLET_DIR ?= src
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(TLSDK_DIR)/trustlet.mk

endif
