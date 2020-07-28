ifneq ($(strip $(MSSI_MTK_TC1_COMMON_SERVICE)), yes)
LOCAL_PATH := $(call my-dir)
VALIDATE_PATH := vendor/mediatek/proprietary/frameworks/opt/appluginmanager/validate

include $(CLEAR_VARS)

include $(call all-makefiles-under, $(LOCAL_PATH))

endif
