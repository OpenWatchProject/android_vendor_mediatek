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

#MODULE_CFLAGS += -D__TRUSTZONE_TEE__

MODULE_COMPILEFLAGS += -I$(LOCAL_DIR)/include \
	-I$(LOCAL_DIR)/../../interface/mtee/include/common/include \
	-Itrusty/lk/trusty/include/uapi/uapi \
	-Itrusty/lk/trusty/include/pal/uapi/uapi/ \
	-I$(LOCAL_DIR)/include/lib/mtee/ \
	-I$(LOCAL_DIR)/../mtee_serv

#GLOBAL_INCLUDES += $(LOCAL_DIR)/include \
                   $(LOCAL_DIR)/../../../vendor/mediatek/geniezone/source/trusty-kernel/include/mtee/common/src/sys

#MODULE_STATIC_LIB := true

MODULE_SRCS += \
	$(LOCAL_DIR)/mtee_api.c \

MODULE_SRCS += \
	$(LOCAL_DIR)/mtee_mem_api.c \
	$(LOCAL_DIR)/mtee_mmap.c \

MODULE_SRCS += \
	$(LOCAL_DIR)/mtee_chn.c \

#	$(LOCAL_DIR)/mtee_irq.c \

MODULE_SRCS += \
	$(LOCAL_DIR)/mtee_sync.c \
	$(LOCAL_DIR)/mtee_time.c \
	$(LOCAL_DIR)/mtee_thread.c \

MODULE_SRCS += \
	$(LOCAL_DIR)/utee_system.c \
	$(LOCAL_DIR)/mtee_drv_api.c \

#	$(LOCAL_DIR)/mtee_sync_sysmem.c \
#	$(LOCAL_DIR)/mtee_sync_sysipc.c \
#	$(LOCAL_DIR)/mtee_sync_system \
#	$(LOCAL_DIR)/mtee_sync_ktee_system.c \
#	$(LOCAL_DIR)/mtee_sync_kta_sysmem.c \
#	$(LOCAL_DIR)/mtee_sync_mteeree.c \

MODULE_SRCS += \
	$(LOCAL_DIR)/mtee_sys_api.c \
	$(LOCAL_DIR)/mtee_location.c \

MODULE_STATIC_LIBS := \
	trusty/lib/lib/libc-trusty \
	trusty/lib/lib/mtee_serv \
	trusty/lib/interface/mtee \

#MODULE_DEPS += \
	lib/libc-trusty \
	lib/mtee_serv \
	interface/mtee \

include make/module.mk
