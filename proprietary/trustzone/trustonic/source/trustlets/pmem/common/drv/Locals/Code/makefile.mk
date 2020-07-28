#
# Copyright (c) 2018 MediaTek Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files
# (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software,
# and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

################################################################################
# <t-sdk pmem driver
################################################################################

# output binary name without path or extension
OUTPUT_NAME := drpmem

#-------------------------------------------------------------------------------
# MobiConvert parameters, see manual for details
#-------------------------------------------------------------------------------
DRIVER_UUID := 08050000000000000000000000003419
DRIVER_MEMTYPE := 2 # 0: iram preferred; 1: iram; 2: eram
DRIVER_NO_OF_THREADS := 4 # min =1; max =8
DRIVER_SERVICE_TYPE := 1 # 1: driver; 2: service provider trustlet; 3: system trustlet
DRIVER_KEYFILE := Locals/Build/pairVendorTltSig.pem
DRIVER_FLAGS:= 0 # 0: no flag; 1: permanent; 2: service has no WSM control interface; 3: both (permanent and service has not WSM control interface)
DRIVER_VENDOR_ID := 9 # Mediatek
DRIVER_NUMBER := 0x3419 # drpmem
DRIVER_ID := $$(($(DRIVER_VENDOR_ID)<<16|$(DRIVER_NUMBER)))
DRIVER_INTERFACE_VERSION_MAJOR=1
DRIVER_INTERFACE_VERSION_MINOR=0
DRIVER_INTERFACE_VERSION := $(DRIVER_INTERFACE_VERSION_MAJOR).$(DRIVER_INTERFACE_VERSION_MINOR)

#-------------------------------------------------------------------------------
# For 302A and later version
#-------------------------------------------------------------------------------
TBASE_API_LEVEL := 5
HEAP_SIZE_INIT := 4096
HEAP_SIZE_MAX := 65536

ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)
EXTRA_LIBS += $(MSEE_FWK_DRV_OUT_DIR)/$(TEE_MODE)/msee_fwk_drv.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/drpmem_core_intermediates/drpmem_core.lib
endif

#-------------------------------------------------------------------------------
# use Trustonic generic make file
#-------------------------------------------------------------------------------
DRIVER_DIR ?= Locals/Code
DRSDK_DIR_SRC ?= $(DRSDK_DIR)
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(DRSDK_DIR)/driver.mk
