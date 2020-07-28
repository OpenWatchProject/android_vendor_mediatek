# Copyright (C) 2014-2015 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_DIR := $(GET_LOCAL_DIR)

GLOBAL_CPPFLAGS += -mfloat-abi=softfp -mfpu=neon
GLOBAL_CFLAGS += -mfloat-abi=softfp -mfpu=neon
GLOBAL_ASMFLAGS += -mfloat-abi=softfp -mfpu=neon
MODULE_CFLAGS   += -O3
MODULE_CPPFLAGS += -O3

ifneq ("$(wildcard $(LOCAL_DIR)/$(VPU_ELF)/vpu_main_sDSP_0)","")
$(shell xxd -i $(LOCAL_DIR)/$(VPU_ELF)/vpu_main_sDSP_0 > $(LOCAL_DIR)/vpu_main_sDSP_0.c)
$(shell sed -e 's/app_sample_fod_sample_$(VPU_ELF)_//' -i $(LOCAL_DIR)/vpu_main_sDSP_0.c)
MODULE_CFLAGS += -DSUPPORT_sDSP_0
endif

ifneq ("$(wildcard $(LOCAL_DIR)/$(VPU_ELF)/vpu_main_sDSP_1)","")
$(shell xxd -i $(LOCAL_DIR)/$(VPU_ELF)/vpu_main_sDSP_1 > $(LOCAL_DIR)/vpu_main_sDSP_1.c)
$(shell sed -e 's/app_sample_fod_sample_$(VPU_ELF)_//' -i $(LOCAL_DIR)/vpu_main_sDSP_1.c)
MODULE_CFLAGS += -DSUPPORT_sDSP_1
endif

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/manifest.c \
	$(LOCAL_DIR)/fod_sample_main.c \
 	$(LOCAL_DIR)/matrix.c

MODULE_DEPS += \
  app/trusty \
	lib/libc-trusty \
	lib/mtee_api \
	lib/mtee_serv

include make/module.mk
