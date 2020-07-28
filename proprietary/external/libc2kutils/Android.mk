#
# Copyright 2009 Cedric Priscal
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
# http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License. 
#

LOCAL_PATH:= $(call my-dir)

# For vendor partion
include $(CLEAR_VARS)
LOCAL_SRC_FILES:= \
    c2kutils.c \
    com_intf.c

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libutils \
    libcutils

LOCAL_PRELINK_MODULE := false
LOCAL_MULTILIB := both
LOCAL_MODULE := libc2kutils
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional

## Note: Suffix will be temp if compile the module by mm in the directory
ifeq ($(strip $(REPO_VERSION)),)
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=\"temp\"
else
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=$(REPO_VERSION)
endif
LOCAL_LDLIBS += -lc
LOCAL_CFLAGS += -DCOM_ANDROID
include $(MTK_SHARED_LIBRARY)

################################################
# For system partion
include $(CLEAR_VARS)
LOCAL_SRC_FILES:= \
    c2kutils.c \
    com_intf.c

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libutils \
    libcutils

LOCAL_PRELINK_MODULE := false
LOCAL_MULTILIB := both
LOCAL_MODULE := libc2kutils_sys
LOCAL_MODULE_TAGS := optional

## Note: Suffix will be temp if compile the module by mm in the directory
ifeq ($(strip $(REPO_VERSION)),)
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=\"temp\"
else
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=$(REPO_VERSION)
endif
LOCAL_LDLIBS += -lc
LOCAL_CFLAGS += -DCOM_ANDROID
include $(MTK_SHARED_LIBRARY)
