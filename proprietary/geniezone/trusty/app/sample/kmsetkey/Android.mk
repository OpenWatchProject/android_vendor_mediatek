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

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := kmsetkey.elf
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(GZ_APP_OUT)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_SRC_FILES := \
	kmsetkey.c \
	cryptolib.c \
	test.c \
	manifest.c

LOCAL_C_INCLUDES := \
	$(GZ_ROOT_DIR)/trusty/app/sample/kmsetkey \
	$(GZ_ROOT_DIR)/../../../../external/boringssl/include

LOCAL_STATIC_LIBRARIES := \
	libc-trusty \
	libc.mod \
	libmtee_boringssl \
	libopenssl_stubs \
	librng

LOCAL_MULTILIB := 32

LOCAL_CFLAGS += -fno-short-enums

include $(GZ_EXECUTABLE)
