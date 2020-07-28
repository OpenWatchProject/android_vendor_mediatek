#
# Copyright (C) 2014 MediaTek Inc.
# Modification based on code covered by the mentioned copyright
# and/or permission notice(s).
#
#
# Copyright (C) 2008 The Android Open Source Project
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
LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
        sockets.cpp \
        strdup16to8.c \
        strdup8to16.c \
        socket_local_client_unix.c \
        socket_local_server_unix.c \
        sockets_unix.cpp \
        native_handle.c \
        sched_policy.c \
        android_get_control_file.cpp \

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/log
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/utils

LOCAL_SHARED_LIBRARIES := libmtkrillog
LOCAL_STATIC_LIBRARIES += libmtkrillog_android_arm

LOCAL_MODULE := libmtkcutils
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_CFLAGS := -Werror -Wall -Wextra
include $(MTK_SHARED_LIBRARY)
