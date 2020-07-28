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

ifeq ($(strip $(MTK_CAM_SECURITY_SUPPORT)), yes)
ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

IMGSENSOR_COMMON_SRC = ../../common

LOCAL_MODULE := libimgsensor_sec_custom
LOCAL_PROPRIETARY_MODULE := true

#include folder
LOCAL_C_INCLUDES += \
    $(LINUX_KERNEL_VERSION)/drivers/misc/mediatek/imgsensor/inc \
    $(LOCAL_PATH)/$(IMGSENSOR_COMMON_SRC)
#include flag
LOCAL_CFLAGS += -DSENSOR_MTK_SECURE_I2C_BUS=$(MTK_SECURE_I2C_BUS)
LOCAL_CFLAGS += -DSENSOR_MTK_SECURE_CSI=$(MTK_SECURE_CSI)

define FILTER_DRV
folders += $(IMGSENSOR_COMMON_SRC)/src/$(1)
endef

define FILTER_FLAG
flag += -D$(shell echo $(1) | tr '[a-z]' '[A-Z]')=1
endef

$(foreach drv,$(subst $\",,$(MTK_SECURE_SENSOR)),$(eval $(call FILTER_DRV,$(drv))))


#include source
LOCAL_SRC_FILES += $(call all-c-files-under,$(folders))
$(info Secure-Sensor LOCAL_SRC_FILES = $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES += $(IMGSENSOR_COMMON_SRC)/imgsensor_sec_custom.c
LOCAL_SRC_FILES += $(IMGSENSOR_COMMON_SRC)/imgsensor_sec_sensorlist.c

#include flag
$(foreach flag_tmp,$(subst $\",,$(MTK_SECURE_SENSOR)),$(eval $(call FILTER_FLAG,$(flag_tmp))))
LOCAL_CFLAGS += $(flag)
$(info Secure-Sensor LOCAL_CFLAGS = $(LOCAL_CFLAGS))

LOCAL_SHARED_LIBRARIES := libTEECommon
LOCAL_SHARED_LIBRARIES += liblog

#make a static lib
include vendor/mediatek/proprietary/trustzone/custom/build/tee_static_library.mk

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreDriverLib))
$(call import-module,$(COMP_PATH_Tlcmem))
$(call import-module,$(COMP_PATH_SecLib))
$(call import-module,$(COMP_PATH_SectraceLib))
#-----------------------------------------------------

endif # ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)
endif
