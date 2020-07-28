################################################################################
#
# <t-sdk ISP Driver
#
################################################################################

# output binary name without path or extension
ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)
OUTPUT_NAME := drimgsensor
endif

#-------------------------------------------------------------------------------
# MobiConvert parameters, see manual for details
#-------------------------------------------------------------------------------
DRIVER_UUID := 07407000000000000000000000000000
DRIVER_MEMTYPE := 2 # 0: iram preferred; 1: iram; 2: eram
DRIVER_NO_OF_THREADS := 4 # min =1; max =8
DRIVER_SERVICE_TYPE := 1 # 1: driver; 2: service provider trustlet; 3: system trustlet
DRIVER_KEYFILE := Locals/Build/pairVendorTltSig.pem
DRIVER_FLAGS:= 0 # 0: no flag; 1: permanent; 2: service has no WSM control interface; 3: both (permanent and service has not WSM control interface)
DRIVER_VENDOR_ID := 9 # MediaTek
DRIVER_NUMBER := 0x90222 # SecSeninfDrv
DRIVER_ID := $$(($(DRIVER_VENDOR_ID)<<16|$(DRIVER_NUMBER)))
DRIVER_INTERFACE_VERSION_MAJOR=1
DRIVER_INTERFACE_VERSION_MINOR=0
DRIVER_INTERFACE_VERSION := $(DRIVER_INTERFACE_VERSION_MAJOR).$(DRIVER_INTERFACE_VERSION_MINOR)

#-------------------------------------------------------------------------------
# DCI handler and trustlet APIs are enabled as default
#-------------------------------------------------------------------------------
USE_DCI_HANDLER := YES
USE_TL_API := YES

#-------------------------------------------------------------------------------
# Files and include paths - Add your files here
#-------------------------------------------------------------------------------

ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)

#-------------------------------------------------------------------------------
# For 302A and later version
#-------------------------------------------------------------------------------
TBASE_API_LEVEL := 5
HEAP_SIZE_INIT := 4096
HEAP_SIZE_MAX := 65536

ifeq ($(strip $(TARGET_BUILD_VARIANT)),eng)
ARMCC_COMPILATION_FLAGS += -DTARGET_BUILD_VARIANT_ENG
endif

ARMCC_COMPILATION_FLAGS += -DDRIVER_ID=$(DRIVER_ID)
ARMCC_COMPILATION_FLAGS += -DDRIVER_VERSION_MAJOR=$(DRIVER_INTERFACE_VERSION_MAJOR)
ARMCC_COMPILATION_FLAGS += -DDRIVER_VERSION_MINOR=$(DRIVER_INTERFACE_VERSION_MINOR)
ARMCC_COMPILATION_FLAGS += -fshort-wchar
ARMCC_SHORT_ENUMS := YES

#-------------------------------------------------------------------------------
# Library to include
#-------------------------------------------------------------------------------
EXTRA_LIBS += $(MSEE_FWK_DRV_OUT_DIR)/$(TEE_MODE)/msee_fwk_drv.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/ImgsensorDriver_intermediates/ImgsensorDriver.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/drsec_api_intermediates/drsec_api.lib
EXTRA_LIBS += $(MSEE_FWK_TA_OUT_DIR)/$(TEE_MODE)/msee_fwk_ta.lib

#-------------------------------------------------------------------------------
# use generic make file
DRIVER_DIR ?= core
DRLIB_DIR := api
DRSDK_DIR_SRC ?= $(DRSDK_DIR)
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(DRSDK_DIR)/driver.mk

endif #$ ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)

