#
# Copyright (c) 2015 MediaTek Inc.
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

TARGET = e97c270ea5c44c58bcd3384a2fa2539e.ta

MTK_RPMB_SUPPORT := no

# Secure memory
#CUSTOMER_DRIVER_LIBS += $(MSEE_FWK_TA_OUT_DIR)/$(TEE_MODE)/msee_fwk_ta.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/drutils_api_intermediates/drutils_api.lib

ifeq ($(MTK_RPMB_SUPPORT), yes)
    HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/drrpmb_gp_api_intermediates/drrpmb_gp_api.lib
endif

HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/drWidevineModularDrm_api_intermediates/drWidevineModularDrm_api.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/taWidevineModularDrm_core_intermediates/taWidevineModularDrm_core.lib
HAL_LIBS += $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/drDRMHDCPCommon_api_intermediates/drDRMHDCPCommon_api.lib

SRC_C += manifest.c

MY_CFLAGS = $(CFLAGS_PF_UT_BTA) $(CFLAGS_PF_UT_DRV_CLIENT)

MY_LDFLAGS = $(LDFLAGS_PF_UT_BTA) $(LDFLAGS_PF_UT_DRV_CLIENT)

MODE = shared

include $(UT_SDK_HOME)/build/prog.mk
