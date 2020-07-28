#
# Copyright (C) 2016 The Android Open Source Project
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


LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.mediatek.hardware.wfo@1.0-service
LOCAL_INIT_RC := vendor.mediatek.hardware.wfo@1.0-service.rc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/*.c*))
LOCAL_C_INCLUDES := $(MTK_PATH_SOURCE)/hardware/ims/include/mal_header/include/

LOCAL_SHARED_LIBRARIES := \
	libbase \
	libcutils \
	liblog \
	libdl \
	libutils \
	libhardware \
	libhidlbase \
	libhidltransport \
	libhwbinder \
	vendor.mediatek.hardware.wfo@1.0

include $(BUILD_EXECUTABLE)
