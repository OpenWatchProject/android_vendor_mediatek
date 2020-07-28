## This is OP16 operator config file
# Set for carrier express features in this File for OP16


ifeq ($(strip $(MTK_CIP_SUPPORT)), yes)
USP_PATH_ROOT := /custom
else
USP_PATH_ROOT := /system
endif
USP_OPERATOR_APK_PATH :=

USP_OPERATOR_PACKAGES := com.mediatek.op16.settingsProvider

USP_OPERATOR_FEATURES := MTK_EPDG_CIP_SUPPORT=yes \
                MTK_WFC_SUPPORT=yes \
                MTK_VOLTE_SUPPORT=yes \
                MSSI_MTK_VILTE_SUPPORT=no