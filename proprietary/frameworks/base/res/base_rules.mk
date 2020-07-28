
# used to change from "hdpi,xhdpi" to "hdpi xhdpi"
#comma:= ,
#empty:=
#space:= $(empty) $(empty)

MTK_RMS_ERROR_MESSAGES :=

# check default overlay path, should be device/mediatek/$(MTK_TARGET_PROJECT)/overlay & device/mediatek/$(MTK_BASE_PROJECT)/overlay
#############################################################
ifeq ( ,$(findstring banyan_x86,$(MTK_TARGET_PROJECT)))
ifneq ( ,$(strip $(MTK_TARGET_PROJECT)))
  ifeq ( ,$(findstring $(MTK_TARGET_PROJECT)/overlay,$(DEVICE_PACKAGE_OVERLAYS)))
     MTK_RMS_ERROR_MESSAGES += Please add device/<company>/$(MTK_TARGET_PROJECT)/overlay to DEVICE_PACKAGE_OVERLAYS;
  endif
endif
endif
ifeq ( ,$(findstring banyan_x86,$(MTK_BASE_PROJECT)))
ifneq ( ,$(strip $(MTK_BASE_PROJECT)))
  ifeq ( ,$(findstring $(MTK_BASE_PROJECT)/overlay,$(DEVICE_PACKAGE_OVERLAYS)))
     MTK_RMS_ERROR_MESSAGES += Please add device/<company>/$(MTK_BASE_PROJECT)/overlay to DEVICE_PACKAGE_OVERLAYS;
  endif
endif
endif


# check MTK_GMO_ROM_OPTIMIZE/MTK_GMO_RAM_OPTIMIZE & DEVICE_PACKAGE_OVERLAYS
#############################################################
ifeq (yes,$(strip $(MTK_GMO_ROM_OPTIMIZE)))
  ifeq ( ,$(filter device/mediatek/common/overlay/slim_rom,$(DEVICE_PACKAGE_OVERLAYS)))
     MTK_RMS_ERROR_MESSAGES += Please add value slim_romto DEVICE_PACKAGE_OVERLAYS or turn off MTK_GMO_ROM_OPTIMIZE;
  endif
endif
ifneq (yes,$(strip $(MTK_GMO_ROM_OPTIMIZE)))
  ifneq ( ,$(filter device/mediatek/common/overlay/slim_rom,$(DEVICE_PACKAGE_OVERLAYS)))
     MTK_RMS_ERROR_MESSAGES += Please remove slim_romfrom DEVICE_PACKAGE_OVERLAYS or turn on MTK_GMO_ROM_OPTIMIZE;
  endif
endif
ifeq (yes,$(strip $(MTK_GMO_RAM_OPTIMIZE)))
  ifeq ( ,$(filter device/mediatek/common/overlay/slim_ram,$(DEVICE_PACKAGE_OVERLAYS)))
    MTK_RMS_ERROR_MESSAGES += Please add value slim_ram to DEVICE_PACKAGE_OVERLAYS or turn off MTK_GMO_RAM_OPTIMIZE;
  endif
endif
ifneq (yes,$(strip $(MTK_GMO_RAM_OPTIMIZE)))
  ifneq ( ,$(filter device/mediatek/common/overlay/slim_ram,$(DEVICE_PACKAGE_OVERLAYS)))
    MTK_RMS_ERROR_MESSAGES += Please remove slim_ram from DEVICE_PACKAGE_OVERLAYS or turn on MTK_GMO_RAM_OPTIMIZE;
  endif
endif

# check qHD & FWVGA
#############################################################
ifneq (yes,$(strip $(MTK_TABLET_PLATFORM)))
ifneq (yes,$(strip $(MTK_BSP_PACKAGE)))
  ifeq (480,$(strip $(LCM_WIDTH)))
    ifeq (854,$(strip $(LCM_HEIGHT)))
      ifeq ( ,$(filter device/mediatek/common/overlay/FWVGA,$(DEVICE_PACKAGE_OVERLAYS)))
        MTK_RMS_ERROR_MESSAGES += Please add value FWVGA to DEVICE_PACKAGE_OVERLAYS or set different LCM_WIDTH and LCM_HEIGHT;
      endif
    endif
  endif
  ifeq (540,$(strip $(LCM_WIDTH)))
    ifeq (960,$(strip $(LCM_HEIGHT)))
      ifeq ( ,$(filter device/mediatek/common/overlay/qHD,$(DEVICE_PACKAGE_OVERLAYS)))
        MTK_RMS_ERROR_MESSAGES += Please add value qHD to DEVICE_PACKAGE_OVERLAYS or set different LCM_WIDTH and LCM_HEIGHT;
      endif
    endif
  endif
endif
endif

# check OPTR_SPEC_SEG_DEF & DEVICE_PACKAGE_OVERLAYS
#############################################################
ifdef OPTR_SPEC_SEG_DEF
  ifneq ($(strip $(OPTR_SPEC_SEG_DEF)),NONE)
    OPTR := $(word 1,$(subst _,$(space),$(OPTR_SPEC_SEG_DEF)))
    SPEC := $(word 2,$(subst _,$(space),$(OPTR_SPEC_SEG_DEF)))
    SEG  := $(word 3,$(subst _,$(space),$(OPTR_SPEC_SEG_DEF)))
    
    ifeq ( ,$(filter device/mediatek/common/overlay/operator/$(OPTR)/$(SPEC)/$(SEG),$(DEVICE_PACKAGE_OVERLAYS)))
      MTK_RMS_ERROR_MESSAGES += Please correct DEVICE_PACKAGE_OVERLAYS or set different OPTR_SPEC_SEG_DEF($(OPTR_SPEC_SEG_DEF));
    endif
  endif
endif


ifneq ( ,$(strip $(MTK_RMS_ERROR_MESSAGES)))
$(info MTK_TARGET_PROJECT=$(MTK_TARGET_PROJECT))
$(info MTK_BASE_PROJECT=$(MTK_BASE_PROJECT))
$(info DEVICE_PACKAGE_OVERLAYS=$(DEVICE_PACKAGE_OVERLAYS))
$(info MTK_GMO_ROM_OPTIMIZE=$(MTK_GMO_ROM_OPTIMIZE))
$(info PRODUCT_AAPT_CONFIG=$(PRODUCT_AAPT_CONFIG))
$(info PRODUCT_AAPT_PREF_CONFIG=$(PRODUCT_AAPT_PREF_CONFIG))
$(info MTK_TABLET_PLATFORM=$(MTK_TABLET_PLATFORM))
$(info LCM_WIDTH=$(LCM_WIDTH))
$(info LCM_HEIGHT=$(LCM_HEIGHT))
$(info OPTR_SPEC_SEG_DEF=$(OPTR_SPEC_SEG_DEF))
$(info MTK_SMARTBOOK_SUPPORT=$(MTK_SMARTBOOK_SUPPORT))
ifneq ( ,$(filter DEVICE_PACKAGE_OVERLAYS,$(MTK_RMS_ERROR_MESSAGES)))
MTK_RMS_ERROR_MESSAGES += DEVICE_PACKAGE_OVERLAYS is set in device/<company>/$(MTK_TARGET_PROJECT)/device.mk;
endif
ifneq ( ,$(filter PRODUCT_AAPT_CONFIG,$(MTK_RMS_ERROR_MESSAGES)))
MTK_RMS_ERROR_MESSAGES += PRODUCT_AAPT_CONFIG is set in device/<company>/$(MTK_TARGET_PROJECT)/full_$(MTK_TARGET_PROJECT).mk;
endif
$(error $(MTK_RMS_ERROR_MESSAGES))
endif
