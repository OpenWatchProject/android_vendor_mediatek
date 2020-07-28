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



LOCAL_SRC_FILES := \
                   ipxfrm_mon.cpp   \
		   xfrm_policy.cpp \
		   ipsec_multilayer.cpp  \
		   xfrm_forward.cpp  \
		   netutils_wrapper_interface.cpp \
		   netlink_route.cpp \
		   xfrm_parse.cpp  \
		   utils_xfrm.cpp

ifneq (,$(filter userdebug eng,$(TARGET_BUILD_VARIANT)))
LOCAL_CPPFLAGS += -DINIT_ENG_BUILD
endif
LOCAL_MODULE := ipsec_mon
LOCAL_MODULE_TAGS := optional
LOCAL_HEADER_LIBRARIES := liblog_headers ipsec_mon_includes netdagent_headers
LOCAL_SHARED_LIBRARIES := libc libcutils liblog libforkexecwrap libifcutils_mtk
  
LOCAL_CFLAGS := -Wall
LOCAL_CFLAGS += -DMOBIKE
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_INIT_RC := ipsec_mon.rc
include $(MTK_EXECUTABLE)

include $(call all-makefiles-under,$(LOCAL_PATH))


