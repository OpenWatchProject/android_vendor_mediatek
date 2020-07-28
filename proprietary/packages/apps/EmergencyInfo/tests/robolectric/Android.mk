# Copyright (C) 2017 The Android Open Source Project
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

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_JAVA_RESOURCE_DIRS := config

LOCAL_STATIC_JAVA_LIBRARIES := \
    platform-robolectric-android-all-stubs \
    mockito-robolectric-prebuilt \
    truth-prebuilt \
    Robolectric_all-target \
    emergencyinfo-test-common_mtk

LOCAL_JAVA_LIBRARIES := \
    junit

LOCAL_INSTRUMENTATION_FOR := MtkEmergencyInfo
LOCAL_MODULE := MtkEmergencyInfoRoboTests

LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_JAVA_LIBRARY)

#############################################################
# EmergencyInfo runner target to run the previous target. #
#############################################################
include $(CLEAR_VARS)

LOCAL_MODULE := MtkRunEmergencyInfoRoboTests

LOCAL_SDK_VERSION := current

LOCAL_STATIC_JAVA_LIBRARIES := \
    MtkEmergencyInfoRoboTests

LOCAL_JAVA_LIBRARIES := \
    EmergencyInfoRoboTests \
    robolectric_android-all-stub \
    Robolectric_all-target \
    mockito-robolectric-prebuilt \
    truth-prebuilt

LOCAL_TEST_PACKAGE := MtkEmergencyInfo

include external/robolectric-shadows/run_robotests.mk
