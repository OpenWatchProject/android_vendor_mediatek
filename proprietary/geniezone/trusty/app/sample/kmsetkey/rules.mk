# Copyright (C) 2013-2014 The Android Open Source Project
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

#BORINGSSL_PATH := $(ANDROIDMAKEROOT)/external/boringssl

MODULE_SRCS += \
	$(LOCAL_DIR)/kmsetkey.c \
	$(LOCAL_DIR)/cryptolib.c \
	$(LOCAL_DIR)/test.c \
	$(LOCAL_DIR)/manifest.c

MODULE_INCLUDES := \
	$(LOCAL_DIR)

MODULE_DEPS += \
	app/trusty \
	lib/libc-trusty \
	lib/boringssl \
	lib/openssl-stubs

MODULE_CFLAGS += -fno-short-enums

include make/module.mk
