
## This is OP03 operator config file

# Set for carrier express features in this File for OP03

ifeq ($(strip $(MTK_CIP_SUPPORT)), yes)
USP_PATH_ROOT := /custom
else
USP_PATH_ROOT := /system
endif

USP_OPERATOR_FEATURES := MTK_EPDG_CIP_SUPPORT=yes \
                MTK_WFC_SUPPORT=yes \
                MTK_VOLTE_SUPPORT=yes \
                MSSI_MTK_VILTE_SUPPORT=no
