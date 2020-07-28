# Copyright (C) 2015 The Android Open Source Project
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
#
# Build rule for Launcher3Tests
#
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := tests
LOCAL_STATIC_JAVA_LIBRARIES := \
        androidx.test.runner \
        androidx.test.rules \
        androidx.test.uiautomator_uiautomator \
        mockito-target-minus-junit4

ifneq (,$(wildcard frameworks/base))
    LOCAL_PRIVATE_PLATFORM_APIS := true
    LOCAL_STATIC_JAVA_LIBRARIES += launcher-aosp-tapl
else
    LOCAL_SDK_VERSION := 28
    LOCAL_MIN_SDK_VERSION := 21
    LOCAL_STATIC_JAVA_LIBRARIES += ub-launcher-aosp-tapl
endif

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_FULL_LIBS_MANIFEST_FILES := $(LOCAL_PATH)/AndroidManifest-common.xml

LOCAL_PACKAGE_NAME := MtkLauncher3Tests

LOCAL_INSTRUMENTATION_FOR := MtkLauncher3QuickStep

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))
