ifeq ($(TRUSTONIC_TEE_SUPPORT),yes)

#define mtk-install-drtl
#include $$(CLEAR_VARS)
#LOCAL_MODULE := $$(shell basename $(1))
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#LOCAL_MODULE_CLASS := ETC
#LOCAL_MODULE_PATH := $(2)
#LOCAL_SRC_FILES := $(1)
#include $$(BUILD_PREBUILT)
#MTK_TRUSTONIC_INSTALLED_MODULES += $$(LOCAL_INSTALLED_MODULE)
#endef

#LOCAL_PATH := $(TOP)
#drbins := $(shell find $(PRODUCT_OUT)/trustzone -type f -name "*.drbin" -print)
#tlbins := $(shell find $(PRODUCT_OUT)/trustzone -type f -name "*.tlbin" -print)

#$(foreach item,$(drbins),$(eval ALL_DEFAULT_INSTALLED_MODULES += $(shell basename $(item))))
#$(foreach item,$(drbins),$(eval $(call mtk-install-drtl,$(item),$(TARGET_OUT_APPS)/mcRegistry)))
#$(foreach item,$(tlbins),$(eval ALL_DEFAULT_INSTALLED_MODULES += $(shell basename $(item))))
#$(foreach item,$(tlbins),$(eval $(call mtk-install-drtl,$(item),$(TARGET_OUT_APPS)/mcRegistry)))

##############################################################
# <t-play
LOCAL_PATH := $(TOP)
-include $(LOCAL_PATH)/vendor/mediatek/proprietary/trustzone/trustonic/source/trustlets/tplay/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/Tlctplay/Android.mk
##############################################################

LOCAL_PATH := $(TOP)
-include $(LOCAL_PATH)/vendor/mediatek/proprietary/protect-bsp/trustzone/trustonic/trustlets/key_install/TlcKeyInstall/Android.mk

LOCAL_PATH := $(TOP)

-include $(LOCAL_PATH)/vendor/mediatek/proprietary/trustzone/trustonic/secure/trustlets/video/platform/mt6735/MtkH264Vdec/MtkH264SecVdecTLC/Locals/Code/Android.mk

LOCAL_PATH := $(TOP)

LOCAL_PATH := $(TOP)

-include $(LOCAL_PATH)/vendor/mediatek/proprietary/trustzone/trustonic/secure/trustlets/video/platform/mt6735/MtkH264Venc/MtkH264SecVencTLC/Locals/Code/Android.mk

ifneq ($(wildcard  $(TOP)/vendor/trustonic/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/export/sectrace/libsectrace.so),)
  ALL_DEFAULT_INSTALLED_MODULES += libsectrace
  include $(CLEAR_VARS)
  LOCAL_MODULE := libsectrace
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
  LOCAL_MODULE_SUFFIX := .so
  LOCAL_MODULE_CLASS := SHARED_LIBRARIES
  LOCAL_MODULE_PATH := $(TARGET_OUT)/lib
  LOCAL_SRC_FILES := $(TOP)/vendor/trustonic/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/export/sectrace/libsectrace.so
  include $(BUILD_PREBUILT)
endif

##############################################################
#Widevine DRM
LOCAL_PATH := $(TOP)
-include $(LOCAL_PATH)/vendor/mediatek/proprietary/trustzone/trustonic/secure/trustlets/widevine/common/modular_drm/TlcWidevineModularDrm/Locals/Code/Android.mk

LOCAL_PATH := $(TOP)
-include $(LOCAL_PATH)/vendor/mediatek/proprietary/trustzone/trustonic/secure/trustlets/widevine/common/classic_drm/TlcWidevineClassicDrm/Locals/Code/Android.mk
##############################################################
LOCAL_PATH := $(TOP)
-include $(LOCAL_PATH)/vendor/trustonic/platform/common/trustlets/rpmb/Tlcrpmb/Locals/Code/Android.mk

LOCAL_PATH := $(TOP)

endif
