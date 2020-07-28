#
# Copyright (C) 2011 The Android Open Source Project
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
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := \
                   src/setkey_fileio/setkey_fileio.c  \
        	   src/setkey_fileio/rtnl_socket.c  \
		   src/setkey_fileio/setkey_xfrm.c  \
		   src/setkey_fileio/setkey_xfrm_parse.c  \
		   src/setkey_fileio/utils_xfrm.c 

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH) \
        $(LOCAL_PATH)/src/setkey_fileio  \


LOCAL_HEADER_LIBRARIES := liblog_headers
LOCAL_SHARED_LIBRARIES := libcutils libcrypto
LOCAL_CFLAGS := -DANDROID_CHANGES -DHAVE_CONFIG_H
LOCAL_CFLAGS += -Wno-sign-compare -Wno-missing-field-initializers
LOCAL_CFLAGS += -Wall

ifneq (,$(filter userdebug eng,$(TARGET_BUILD_VARIANT)))
LOCAL_CFLAGS += -DINIT_ENG_BUILD
endif
LOCAL_MODULE := libipsec_ims
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(MTK_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := \
                   src/setkey_fileio/setkey_fileio.c  \
        	   src/setkey_fileio/rtnl_socket.c  \
		   src/setkey_fileio/setkey_xfrm.c  \
		   src/setkey_fileio/setkey_xfrm_parse.c  \
		   src/setkey_fileio/utils_xfrm.c   \

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH) \
        $(LOCAL_PATH)/src/setkey_fileio  \


LOCAL_HEADER_LIBRARIES := liblog_headers
LOCAL_SHARED_LIBRARIES := libcutils libcrypto liblog

LOCAL_CFLAGS := -DANDROID_CHANGES -DHAVE_CONFIG_H
LOCAL_CFLAGS += -Wno-sign-compare -Wno-missing-field-initializers
LOCAL_CFLAGS += -Wall
ifneq (,$(filter userdebug eng,$(TARGET_BUILD_VARIANT)))
LOCAL_CFLAGS += -DINIT_ENG_BUILD
endif

LOCAL_MODULE := libipsec_ims_shr
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(MTK_SHARED_LIBRARY)


include $(CLEAR_VARS)


LOCAL_SRC_FILES := \
                   test.c 



LOCAL_MODULE := test_xfrm

LOCAL_MODULE_TAGS := optional
LOCAL_HEADER_LIBRARIES := liblog_headers
LOCAL_SHARED_LIBRARIES := libc libcutils liblog libipsec_ims_shr


LOCAL_C_INCLUDES := $(LOCAL_PATH)/src     \
                     $(LOCAL_PATH)/src/setkey_fileio  

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(MTK_EXECUTABLE)

include $(CLEAR_VARS)


include $(call first-makefiles-under,$(LOCAL_PATH)/src)

