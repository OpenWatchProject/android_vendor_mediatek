ifneq ($(wildcard $(LKMAKEROOT)/../modules/trusty-kernel/trng),)
MODULES += $(LKMAKEROOT)/../modules/trusty-kernel/trng
else
MODULES += $(LKMAKEROOT)/vendor/mediatek/geniezone/prebuilt/trusty-kernel/trng
endif

ifneq ($(wildcard $(LKMAKEROOT)/../modules/trusty-kernel/rpmb),)
MODULES += $(LKMAKEROOT)/../modules/trusty-kernel/rpmb
else
MODULES += $(LKMAKEROOT)/vendor/mediatek/geniezone/prebuilt/trusty-kernel/rpmb
endif

ifneq ($(wildcard $(LKMAKEROOT)/../../../security/asf/trusty/kernel/hacc/rules.mk),)
$(info "==== Source SEJ makefiles included. ====")
MODULES += $(LKMAKEROOT)/../../../security/asf/trusty/kernel/hacc
else ifneq ($(wildcard $(LKMAKEROOT)/vendor/mediatek/geniezone/prebuilt/trusty-kernel/hacc/rules.mk),)
$(info "==== Prebuilt SEJ makefiles included. ====")
MODULES += $(LKMAKEROOT)/vendor/mediatek/geniezone/prebuilt/trusty-kernel/hacc
else
$(error "==== No SEJ makefiles included. ====")
endif
