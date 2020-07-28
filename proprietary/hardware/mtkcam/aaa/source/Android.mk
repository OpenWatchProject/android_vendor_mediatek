# Copyright (C) 2008 The Android Open Source Project
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


ifeq ($(TARGET_BOARD_PLATFORM), mt6799)
#$(warning $(TARGET_BOARD_PLATFORM) $(MTK_CAM_SW_VERSION))
ifeq ($(strip $(MTK_CAM_SW_VERSION)),ver2)
include $(call all-named-subdir-makefiles,common mt6799p)
else
include $(call all-named-subdir-makefiles,common mt6799)
endif

else ifeq ($(TARGET_BOARD_PLATFORM), mt6757)
ifeq ($(strip $(MTK_CAM_SW_VERSION)),ver2)
include $(call all-named-subdir-makefiles,common mt6757p)
else
include $(call all-named-subdir-makefiles,common mt6757)
endif

else ifeq ($(TARGET_BOARD_PLATFORM), mt6771)
include $(call all-named-subdir-makefiles,common isp_50)

else ifeq ($(TARGET_BOARD_PLATFORM), mt6775)
include $(call all-named-subdir-makefiles,common isp_50)

else ifeq ($(TARGET_BOARD_PLATFORM), mt6785)
include $(call all-named-subdir-makefiles,common isp_50)

else ifeq ($(TARGET_BOARD_PLATFORM), mt6779)
include $(call all-named-subdir-makefiles,common isp_60)

else ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6883 mt6885))
include $(call all-named-subdir-makefiles,common isp_6s)

else ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6765 mt6768))
    ifeq ($(CAMERA_HAL_VERSION), 3)
        # Hal3
        include $(call all-named-subdir-makefiles,common isp_40)
    else
        # Hal1
        include $(call all-named-subdir-makefiles,common $(TARGET_BOARD_PLATFORM))
    endif

else ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6739 mt6761))
include $(call all-named-subdir-makefiles,common isp_30)

else
include $(call all-named-subdir-makefiles,common $(TARGET_BOARD_PLATFORM))
endif
