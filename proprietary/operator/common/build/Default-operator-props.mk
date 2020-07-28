##
# Here to overide system properties for default operator, current feature options are saved in local variable MCE_OPERATOR_FPS
# with "!" with prefix in values sothat default value for undefined or empty values can be restored correctly.
# while restoring feature options, this character "!" is removed
##

CIP_PROPERTY_OVERRIDES :=
MCE_OPERATOR_FP_NAME :=
MCE_OPERATOR_FPS :=

ifdef MSSI_MTK_CARRIEREXPRESS_PACK
  ifneq ($(strip $(MSSI_MTK_CARRIEREXPRESS_PACK)),no)
    ifdef OPTR_SPEC_SEG_DEF
      ifneq ($(strip $(OPTR_SPEC_SEG_DEF)),NONE)
        include vendor/mediatek/proprietary/operator/common/build/UspOperatorConfig.mk
        # operator features are evaluated here
        $(foreach x,$(USP_OPERATOR_FEATURES), \
            $(eval MCE_OPERATOR_FP_NAME := $(word 1,$(subst =,$(space),$(x)))) \
            $(eval MCE_OPERATOR_FPS += $(MCE_OPERATOR_FP_NAME):=!$(strip $($(MCE_OPERATOR_FP_NAME)))) \
            $(eval $(x)))

        include vendor/mediatek/proprietary/operator/common/build/CIP-Properties.mk
        # Restore features here
        $(foreach x,$(MCE_OPERATOR_FPS), $(eval $(subst !,,$(x))))

        # Override properties for default operator
        PRODUCT_PROPERTY_OVERRIDES += $(CIP_PROPERTY_OVERRIDES)
      endif
    endif
  endif
endif