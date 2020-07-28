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

MODULE_CFLAGS += -D__TRUSTZONE_TEE__

MODULE_CFLAGS   += -O3
MODULE_CPPFLAGS += -O3

GLOBAL_INCLUDES += $(LOCAL_DIR)/include \
                   $(LOCAL_DIR)/../../../vendor/mediatek/geniezone/source/trusty-kernel/include/mtee/common/src/sys

MODULE_STATIC_LIB := true

MODULE_SRCS += \
	$(LOCAL_DIR)/mtee_api.c \
	$(LOCAL_DIR)/mtee_mem_api.c \
	$(LOCAL_DIR)/mtee_mmap.c \
	$(LOCAL_DIR)/mtee_chn.c \
	$(LOCAL_DIR)/mtee_irq.c \
	$(LOCAL_DIR)/mtee_sync.c \
	$(LOCAL_DIR)/mtee_time.c \
	$(LOCAL_DIR)/mtee_thread.c \
	$(LOCAL_DIR)/utee_system.c \
	$(LOCAL_DIR)/mtee_drv_api.c \
	$(LOCAL_DIR)/mtee_sync_sysmem.c \
	$(LOCAL_DIR)/mtee_sync_sysipc.c \
	$(LOCAL_DIR)/mtee_sync_system \
	$(LOCAL_DIR)/mtee_sync_ktee_system.c \
	$(LOCAL_DIR)/mtee_sync_kta_sysmem.c \
	$(LOCAL_DIR)/mtee_sync_mteeree.c \
	$(LOCAL_DIR)/mtee_sys_api.c \
	$(LOCAL_DIR)/mtee_location.c \

ifeq ($(CFG_GZ_SECURE_DSP), 1)
MODULE_SRCS += $(LOCAL_DIR)/mtee_sapu_api.c
endif

MODULE_DEPS += \
	lib/libc-trusty \
	lib/mtee_serv \
	interface/mtee \

include make/module.mk
