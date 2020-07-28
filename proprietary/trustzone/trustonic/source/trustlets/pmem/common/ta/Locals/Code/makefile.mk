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
# <t-sdk pmem GP TA
################################################################################

# output binary name without path or extension
OUTPUT_NAME := tapmem

#-------------------------------------------------------------------------------
# MobiConvert parameters, see manual for details
#-------------------------------------------------------------------------------
TRUSTLET_UUID := 08040000000000000000000000003419
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

ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)
GP_ENTRYPOINTS := Y
CUSTOMER_DRIVER_LIBS += $(MSEE_FWK_TA_OUT_DIR)/$(TEE_MODE)/msee_fwk_ta.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/drpmem_api_intermediates/drpmem_api.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/tapmem_core_intermediates/tapmem_core.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/drutils_api_intermediates/drutils_api.lib
endif

#-------------------------------------------------------------------------------
# use Trustonic generic make file
#-------------------------------------------------------------------------------
TRUSTLET_DIR ?= Locals/Code
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(TLSDK_DIR)/trustlet.mk
