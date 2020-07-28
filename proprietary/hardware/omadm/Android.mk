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
LOCAL_MODULE := libpal
LOCAL_INIT_RC := vendor.mediatek.hardware.omadm@1.0-service.rc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_SRC_FILES := dmclient-helper/service_api/service_api_impl.cpp
LOCAL_SRC_FILES += dmclient-helper/service_api/service_listner_impl.cpp
LOCAL_SRC_FILES += dmclient-helper/pal/pal.cpp
LOCAL_SRC_FILES += dmclient-helper/pal/pal_devdetail.cpp
LOCAL_SRC_FILES += dmclient-helper/pal/pal_connmo.cpp
LOCAL_SRC_FILES += dmclient-helper/pal/pal_admin_apn.cpp
LOCAL_SRC_FILES += dmclient-helper/pal/pal_network.cpp
LOCAL_SRC_FILES += dmclient-helper/pal/pal_devinfo.cpp
LOCAL_SRC_FILES += dmclient-helper/pal/pal_dcmo.cpp
LOCAL_SRC_FILES += dmclient-helper/pal/pal_event_handler.cpp
LOCAL_SRC_FILES += dmclient-helper/pal/pal_event_listener.cpp
LOCAL_SRC_FILES += dmclient-helper/pal/pal_network_status.cpp
LOCAL_SRC_FILES += dmclient-helper/pal/pal_utils.c

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
  libbinder \
  vendor.mediatek.hardware.omadm@1.0

LOCAL_SHARED_LIBRARIES += libc++ libutils libbinder liblog libcutils libcurl libcrypto libexpat libandroid_net

OMADM_C_INCLUDES := $(LOCAL_PATH)/../../../../../frameworks/base/include/ \
                    $(LOCAL_PATH)/../../../../../system/netd/include/ \
                    $(LOCAL_PATH)/../../../../../bionic/libc/dns/include/ \
                    $(LOCAL_PATH)/../../../../../external/libcxx/include/ \
                    $(LOCAL_PATH)/../../../../../external/curl/include/ \
                    $(LOCAL_PATH)/../../../../../external/boringssl/include/ \
                    $(LOCAL_PATH)/../../../../../external/expat/lib/ \

LOCAL_C_INCLUDES := $(LOCAL_PATH)/dmclient-helper/pal $(LOCAL_PATH)/dmclient-helper/include \
                    $(LOCAL_PATH)/dmclient/_include/ \

LOCAL_C_INCLUDES += $(OMADM_C_INCLUDES)

LOCAL_CLANG := true

#LOCAL_CPPFLAGS := \
#        -std=c++11 \

include $(BUILD_SHARED_LIBRARY)

# Create folder for prebuilt proprietary verizon libs and init files
ifeq ($(strip $(MTK_OMADM_SUPPORT)),yes)
    $(shell mkdir -p --mode=755 $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/../operator/verizon/dmclient/lib;)
    $(shell mkdir -p --mode=755 $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/../operator/verizon/dmclient/init;)
    #$(shell mkdir -p --mode=755 $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/../../data/vendor/verizon/dmclient/data;)
endif

