# Copyright 2015 The Android Open Source Project
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

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

ENABLE_ADV_SERVER := no
ADV_CAM_SUPPORT := no

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    liblog \
    libbinder

ifeq ($(ADV_CAM_SUPPORT), yes)
LOCAL_CFLAGS += -DMTKCAM_ADV_CAM_SUPPORT
ENABLE_ADV_SERVER := yes
LOCAL_SHARED_LIBRARIES += libadvcamservice
endif # If advcam support

ifeq ($(strip $(MTK_CAM_MMSDK_SUPPORT)), yes)
    ifneq ($(MTK_CAM_HAL_VERSION),)
        ifeq ($(strip $(MTK_CAM_HAL_VERSION)), 1)
            LOCAL_CFLAGS += -DMTKCAM_MMSDK_SUPPORT
            ENABLE_ADV_SERVER := yes
            LOCAL_SHARED_LIBRARIES += libmmsdkservice
        endif
    endif
endif # If mmsdk support

# Only AdvCam or mmsdk support need to run this process
ifeq ($(ENABLE_ADV_SERVER), yes)
LOCAL_SRC_FILES:= \
    main_advcamserver.cpp

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/frameworks/av/services/mmsdk/include/

LOCAL_MODULE:= mtk_advcamserver
LOCAL_32_BIT_ONLY := true

LOCAL_CFLAGS += -Wall -Wextra -Werror -Wno-unused-parameter

LOCAL_INIT_RC := mtk_advcamserver.rc

include $(BUILD_EXECUTABLE)
endif # If adv_camserver support
