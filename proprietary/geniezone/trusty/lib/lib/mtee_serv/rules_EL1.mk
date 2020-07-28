# Copyright (C) 2014 The Android Open Source Project
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

MODULE := $(LOCAL_DIR)

MODULE_TYPE := trustylib

#MODULE_STATIC_LIB := true

#MODULE_CFLAGS += -D__TRUSTZONE_TEE__

MODULE_COMPILEFLAGS += -I$(LOCAL_DIR)/include \
	-I$(LOCAL_DIR)/../../interface/mtee/include/common/include \
	-Itrusty/lk/trusty/include/pal/shared/lk \
	-Itrusty/lk/trusty/include/pal/uapi/uapi/ \
	-I$(LOCAL_DIR)/


#GLOBAL_INCLUDES += $(LOCAL_DIR)\

MODULE_SRCS += \
	$(LOCAL_DIR)/gp_param.c \
	$(LOCAL_DIR)/mtee_server.c \

MODULE_STATIC_LIBS := \
	trusty/lib/lib/libc-trusty \
	trusty/lib/lib/mtee_api

#MODULE_DEPS += \
	lib/libc-trusty \

include make/module.mk
