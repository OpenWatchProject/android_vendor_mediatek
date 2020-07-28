LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

subdirs := $(addprefix $(LOCAL_PATH)/,$(addsuffix /Android.mk, \
		libfile_op \
		nvram_daemon \
		$(MTK_PLATFORM_DIR) \
	))

include $(subdirs)

ifneq ($(strip $(MTK_BASIC_PACKAGE)),yes)
  include $(MTK_PATH_SOURCE)/external/libnvram/dummy_platform/Android.mk
  include $(MTK_PATH_SOURCE)/external/libnvram/libnvram_sec/Android.mk
#  include $(MTK_PATH_SOURCE)/external/libnvram/nvram_agent_binder/Android.mk
endif

#all branch need build nvramagentclient
include $(MTK_PATH_SOURCE)/external/libnvram/nvramagentclient/Android.mk

#enable nvram hidl
include $(MTK_PATH_SOURCE)/external/libnvram/nvram_hidl/1.1/Android.mk

