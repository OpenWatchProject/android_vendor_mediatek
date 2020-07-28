LOCAL_PATH := $(call my-dir)

#######################
# MTK Version Control #
#######################
include $(CLEAR_VARS)
ifeq ($(strip $(MTK_OFFICIAL_BUILD)), yes)
LOCAL_GIT_SHA := $(shell cd $(LOCAL_PATH); git rev-parse HEAD;)
$(info "MTK_OFFICIAL_BUILD: Update GIT HASH")
endif
ECHO_GIT_SHA := $(shell echo '"$(LOCAL_GIT_SHA)"' > $(LOCAL_PATH)/__nn_hash.inc)