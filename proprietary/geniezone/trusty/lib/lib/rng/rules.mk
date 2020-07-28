# 
# Copyright (C) 2015 MediaTek Inc. 
#
# Modification based on code covered by the below mentioned copyright
# and/or permission notice(S). 
#

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

MODULE := $(LOCAL_DIR)

MODULE_SRCS := $(LOCAL_DIR)/trusty_rng.c

GLOBAL_INCLUDES += $(LOCAL_DIR)/include/

PLATFORM_SSL_PATH := $(ANDROIDMAKEROOT)/external/boringssl
$(info PLATFORM_SSL_PATH = $(PLATFORM_SSL_PATH))

MODULE_DEPS := \
	interface/hwrng \
	lib/libc-trusty \
	$(PLATFORM_SSL_PATH)

include make/module.mk
