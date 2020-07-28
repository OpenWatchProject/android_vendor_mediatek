#
# Copyright (C) 2010 The Android Open Source Project
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
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    com_mediatek_ap15_drm_RecoveryOmaDrm.cpp

LOCAL_C_INCLUDES += \
    $(base)/drm/libdrmframework/include \
    $(base)/drm/libdrmframework/plugins/common/include \
    $(base)/drm/libdrmframework/plugins/common/util/include \
    $(base)/include \
    $(TOP)/frameworks/native/include \
    $(JNI_H_INCLUDE) \
    bionic \
    external/libxml2/include \
    $(LOCAL_PATH)/include \
    $(MTK_PATH_SOURCE)/external/nvram/libnvram \
    $(MTK_PATH_SOURCE)/external/nvram/nvram_agent_binder \
    $(MTK_PATH_SOURCE)/external/nvram/nvramagentclient \
    $(MTK_PATH_SOURCE)/frameworks/av/libdrm/common/mtkutil/include \
    $(MTK_PATH_SOURCE)/frameworks/av/drm/include/drm

LOCAL_SHARED_LIBRARIES := \
    libandroid_runtime \
    libdrmframework \
    libutils \
    libdrmmtkutil \
    libcrypto \
    libnativehelper \
    libbinder \
    libdl \
    libcutils

LOCAL_MODULE:= librecoveryomadrm_jni
LOCAL_MODULE_OWNER := mtk

LOCAL_MODULE_TAGS := optional

include $(MTK_SHARED_LIBRARY)
