# Copyright (c) 2015 MediaTek Inc.

LOCAL_PATH := $(strip $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST)))))

ifneq (,$(MTK_BASE_PROJECT))
LOCAL_BASE_CLEANSPECS := $(shell build/tools/findleaves.py $(FIND_LEAVES_EXCLUDES) $(LOCAL_PATH)/$(MTK_BASE_PROJECT) CleanSpec.mk)
else
LOCAL_BASE_CLEANSPECS :=
endif

ifneq ($(strip $(MTK_PLATFORM)),)
LOCAL_PLATFORM_CLEANSPECS := $(shell build/tools/findleaves.py $(FIND_LEAVES_EXCLUDES) $(LOCAL_PATH)/$(shell echo $(MTK_PLATFORM) | tr '[A-Z]' '[a-z]') CleanSpec.mk)
else
LOCAL_PLATFORM_CLEANSPECS :=
endif

ifneq ($(MTK_EMULATOR_SUPPORT), yes)
include $(LOCAL_BASE_CLEANSPECS) $(LOCAL_PLATFORM_CLEANSPECS)
else
include $(LOCAL_BASE_CLEANSPECS)
endif
