# Copyright 2008, The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

ifeq ($(strip $(MTK_AUTO_TEST)), yes)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_JAVA_LIBRARIES := android.test.runner

LOCAL_STATIC_JAVA_LIBRARIES := junit legacy-android-test

# Include all test java files.
#LOCAL_STATIC_JAVA_LIBRARIES := libjunitreprot-for-omacp-tests
LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := OmacpTests
LOCAL_SDK_VERSION := system_current
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_JAVA_LIBRARIES += telephony-common
LOCAL_STATIC_JAVA_LIBRARIES += android-support-v4

LOCAL_INSTRUMENTATION_FOR := Omacp

LOCAL_MODULE_TAGS := optional

include $(BUILD_PACKAGE)

###############
# Junit Test option
#include $(CLEAR_VARS)
#LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := libjunitreprot-for-omacp-tests:android-junit-report-1.2.6.jar
#include $(BUILD_MULTI_PREBUILT)
###############

endif
