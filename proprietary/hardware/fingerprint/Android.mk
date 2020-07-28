#
# Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
# All Rights Reserved.
#
# Copyright (C) 2013 The Android Open Source Project
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

ifeq ($(strip $(MTK_FINGERPRINT_SUPPORT)),yes)
	ifeq ($(strip $(MTK_FINGERPRINT_SELECT)),$(filter $(MTK_FINGERPRINT_SELECT), GF316M GF318M GF3118M GF518M GF5118M GF516M GF3208 GF3206 GF3266 GF3288 GF5206 GF5216 GF5208))
		LOCAL_PATH := $(call my-dir)

		include $(CLEAR_VARS)

		include $(call all-makefiles-under,$(LOCAL_PATH))
	endif
endif
