#
# Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
# All Rights Reserved.
#

# Oswego series:GF316M/GF318M/GF3118M/GF518M/GF5118M/GF516M/GF816M
# Milan E/F/G/L/FN/K series:GF3266/GF3208/GF3206/GF3288/GF3208FN/GF3228/
# Milan J/H   series: GF3226/GF3258
# Milan HV    series: GF8206/GF6226/GF5288
# Milan A/B/C series: GF5206/GF5216/GF5208. etc

LOCAL_PATH := $(call my-dir)

#auto modify it in script(release_for_xxxx.sh)
FACTORY_TEST:=ree

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

ifeq (x$(FACTORY_TEST), xree)
LOCAL_MODULE := libgf_ta_ree
else
LOCAL_MODULE := libgf_ta
endif

ifeq ($(TARGET_BUILD_VARIANT),eng)
MODE_NAME:=debug
else ifeq ($(TARGET_BUILD_VARIANT),userdebug)
MODE_NAME:=userdebug
else
MODE_NAME:=release
endif

ifeq ($(MTK_FINGERPRINT_SELECT), $(filter $(MTK_FINGERPRINT_SELECT), GF5216))
	LOCAL_SRC_FILES_64 := $(MODE_NAME)/milan_a/arm64-v8a/$(LOCAL_MODULE).so
endif

LOCAL_SHARED_LIBRARIES := liblog
LOCAL_MULTILIB := 64
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
include $(BUILD_PREBUILT)

