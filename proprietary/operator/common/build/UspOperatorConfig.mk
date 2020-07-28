##
# Here set for particular operators features for dynamic configuration
##

#reset all features here with default value: we should use new feature options other than used in projectconfig.mk file otherwise there will conflict with global vaiables whose purpose is different
#Since these variables are used only for property overridden under CIP

## This is default operator config file

# Set for carrier express default values
USP_OPERATOR_APK_PATH :=
USP_OPERATOR_PACKAGES :=
USP_OPERATOR_FEATURES := MTK_EPDG_CIP_SUPPORT=yes \
                MTK_WFC_SUPPORT=yes \
                MTK_VOLTE_SUPPORT=yes \
                MSSI_MTK_VILTE_SUPPORT=yes \
                MSSI_MTK_VIWIFI_SUPPORT=yes \
                MTK_DTAG_DUAL_APN_SUPPORT=no \
                MTK_RCS_UA_SUPPORT=no \
                MTK_RCS_SUPPORT=no \
                MTK_RTT_SUPPORT=no \
                MSSI_MTK_UCE_SUPPORT=no \
                MTK_DIGITS_SUPPORT=no \

ifdef OPTR_SPEC_SEG_DEF
	ifeq ($(strip $(OPTR_SPEC_SEG_DEF)),NONE)
		# OPTR_SPEC_SEG_DEF is NONE
		# Check (MTK_TARGET_PROJECT).mk & (TARGET_BOARD_PLATFORM).mk) for Open Market
		ifneq ("$(wildcard vendor/mediatek/proprietary/operator/common/build/$(MTK_TARGET_PROJECT).mk)","")
		-include vendor/mediatek/proprietary/operator/common/build/$(MTK_TARGET_PROJECT).mk
		else ifneq ("$(wildcard vendor/mediatek/proprietary/operator/common/build/$(TARGET_BOARD_PLATFORM).mk)","")
		-include vendor/mediatek/proprietary/operator/common/build/$(TARGET_BOARD_PLATFORM).mk
		endif
	endif
endif
