################################################################################
#
# <t-sdk Key Install Trustlet
#
################################################################################


# output binary name without path or extension
ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)
OUTPUT_NAME := taKeyInstall
GP_ENTRYPOINTS := Y
else
OUTPUT_NAME := TlKeyInstall
endif # MTK_TEE_GP_SUPPORT = yes


#-------------------------------------------------------------------------------
# MobiConvert parameters, see manual for details
#-------------------------------------------------------------------------------
TRUSTLET_UUID := 08110000000000000000000000000000
TRUSTLET_MEMTYPE := 2
TRUSTLET_NO_OF_THREADS := 1
TRUSTLET_SERVICE_TYPE := 3 # 2: service provider trustlet; 3: system trustlet
TRUSTLET_KEYFILE := pairVendorTltSig.pem            #only valid for service provider trustlets.
TRUSTLET_FLAGS := 0
TRUSTLET_INSTANCES := 10

#-------------------------------------------------------------------------------
# For 302A and later version
#-------------------------------------------------------------------------------
TBASE_API_LEVEL := 5
#4K
HEAP_SIZE_INIT := 4096
#10M
HEAP_SIZE_MAX := 65536

#-------------------------------------------------------------------------------
# Files and include paths - Add your files here
#-------------------------------------------------------------------------------
SRC_C += $(firstword $(wildcard drm/$(MTK_PROJECT)/key.c drm/$(ARCH_MTK_PLATFORM)/key.c) drm/common/key.c)

# Secure memory
CUSTOMER_DRIVER_LIBS += $(MSEE_FWK_TA_OUT_DIR)/$(TEE_MODE)/msee_fwk_ta.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/drutils_api_intermediates/drutils_api.lib

ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)

HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/taKeyInstall_core_intermediates/taKeyInstall_core.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/drKeyInstall_api_intermediates/drKeyInstall_api.lib

#-------------------------------------------------------------------------------
# use generic make file
TRUSTLET_DIR ?= .
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(TLSDK_DIR)/trustlet.mk

else # MTK_TEE_GP_SUPPORT = yes

CUSTOMER_DRIVER_LIBS += $(TLKEYINSTALL_OUT_DIR)/$(TEE_MODE)/TlKeyInstall.lib
CUSTOMER_DRIVER_LIBS += $(DRKEYINSTALL_OUT_DIR)/$(TEE_MODE)/DrKeyInstall.lib
#-------------------------------------------------------------------------------
# use generic make file
TRUSTLET_DIR ?= Locals/Code
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(TLSDK_DIR)/trustlet.mk

endif # MTK_TEE_GP_SUPPORT = yes
