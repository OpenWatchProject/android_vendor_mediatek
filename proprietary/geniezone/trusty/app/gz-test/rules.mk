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

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/manifest.c \
	$(LOCAL_DIR)/main.c \
	$(LOCAL_DIR)/mtee_rtc_app.c \
	$(LOCAL_DIR)/shared_mem_HAs_copy.c \
	$(LOCAL_DIR)/test.cpp \
	$(LOCAL_DIR)/fptest_arm.S \
	$(LOCAL_DIR)/test_storage.c

MODULE_DEPS += \
	app/trusty \
	lib/libc-trusty \
	lib/mtee_api \
	lib/mtee_serv \
	lib/libcxx-mtee \
	lib/storage

MULTICORE_TEST := 0
ifeq ($(MULTICORE_TEST), 1)
MODULE_CPPFLAGS += -DMULTICORE_TEST
MODULE_CFLAGS += -DMULTICORE_TEST

MODULE_SRCS += $(LOCAL_DIR)/test_omp.c
endif

include make/module.mk
