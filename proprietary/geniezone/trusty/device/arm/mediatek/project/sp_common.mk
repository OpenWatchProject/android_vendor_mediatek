include project/common.mk

MODULE_SRC_PATH := $(ANDROIDMAKEROOT)/vendor/mediatek/proprietary/trustzone/trusty/modules
MODULE_ASF_PATH := $(ANDROIDMAKEROOT)/vendor/mediatek/proprietary/security/asf/trusty
MODULE_PRIVATE_PATH := $(ANDROIDMAKEROOT)/vendor/mediatek/proprietary/trustzone/trusty/hardware
MODULE_PREBUIT_PATH := $(LKMAKEROOT)/vendor/mediatek/geniezone/prebuilt

TRNG_SOURCE := $(MODULE_SRC_PATH)/trusty-kernel/trng
TRNG_PREBUILT := $(MODULE_PREBUIT_PATH)/trusty-kernel/trng
$(eval $(call mt_module_select,$(TRNG_SOURCE),$(TRNG_PREBUILT)))

RPMB_SOURCE := $(MODULE_SRC_PATH)/trusty-kernel/rpmb
RPMB_PREBUILT := $(MODULE_PREBUIT_PATH)/trusty-kernel/rpmb
$(eval $(call mt_module_select,$(RPMB_SOURCE),$(RPMB_PREBUILT)))

HACC_SOURCE := $(MODULE_ASF_PATH)/kernel/hacc
HACC_PREBUILT := $(MODULE_PREBUIT_PATH)/trusty-kernel/hacc
$(eval $(call mt_module_select,$(HACC_SOURCE),$(HACC_PREBUILT)))

ifneq ($(CFG_CRYPTO_MODULE),HACC)
GCPU_SOURCE := $(MODULE_PRIVATE_PATH)/sec_drv/gcpu
GCPU_PREBUILT := $(MODULE_PREBUIT_PATH)/trusty-kernel/gcpu
$(eval $(call mt_module_select,$(GCPU_SOURCE),$(GCPU_PREBUILT)))
endif # !HACC
