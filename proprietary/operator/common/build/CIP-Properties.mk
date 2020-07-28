##
# Here properties overriden
##
CIP_PROPERTY_OVERRIDES :=

#VoLTE/IMS support
ifeq ($(strip $(MTK_VOLTE_SUPPORT)), yes)
  CIP_PROPERTY_OVERRIDES += persist.vendor.volte_support=1
  CIP_PROPERTY_OVERRIDES += persist.vendor.mtk.volte.enable=1
else ifeq ($(strip $(MTK_VOLTE_SUPPORT)), no)
  CIP_PROPERTY_OVERRIDES += persist.vendor.volte_support=0
  CIP_PROPERTY_OVERRIDES += persist.vendor.mtk.volte.enable=0
endif

ifeq ($(strip $(MTK_WFC_SUPPORT)), yes)
  CIP_PROPERTY_OVERRIDES += persist.vendor.mtk_wfc_support=1
  CIP_PROPERTY_OVERRIDES += persist.vendor.mtk.wfc.enable=1
else ifeq ($(strip $(MTK_WFC_SUPPORT)), no)
  CIP_PROPERTY_OVERRIDES += persist.vendor.mtk_wfc_support=0
  CIP_PROPERTY_OVERRIDES += persist.vendor.mtk.wfc.enable=0
endif

ifeq ($(strip $(MSSI_MTK_VILTE_SUPPORT)),yes)
  CIP_PROPERTY_OVERRIDES += persist.vendor.vilte_support=1
  CIP_PROPERTY_OVERRIDES += persist.vendor.mtk.ims.video.enable=1
else ifeq ($(strip $(MSSI_MTK_VILTE_SUPPORT)), no)
  CIP_PROPERTY_OVERRIDES += persist.vendor.vilte_support=0
  CIP_PROPERTY_OVERRIDES += persist.vendor.mtk.ims.video.enable=0
endif

ifeq ($(strip $(MSSI_MTK_VIWIFI_SUPPORT)),yes)
  CIP_PROPERTY_OVERRIDES += persist.vendor.viwifi_support=1
else
  CIP_PROPERTY_OVERRIDES += persist.vendor.viwifi_support=0
endif

ifeq ($(strip $(MTK_RCS_UA_SUPPORT)),yes)
  CIP_PROPERTY_OVERRIDES += persist.vendor.mtk_rcs_ua_support=1
else ifeq ($(strip $(MTK_RCS_UA_SUPPORT)),no)
  CIP_PROPERTY_OVERRIDES += persist.vendor.mtk_rcs_ua_support=0
endif

ifeq ($(strip $(MSSI_MTK_UCE_SUPPORT)),yes)
  CIP_PROPERTY_OVERRIDES += persist.vendor.mtk_uce_support=1
else ifeq ($(strip $(MSSI_MTK_UCE_SUPPORT)),no)
  CIP_PROPERTY_OVERRIDES += persist.vendor.mtk_uce_support=0
endif

ifeq ($(strip $(MTK_RCS_SUPPORT)),yes)
  CIP_PROPERTY_OVERRIDES += persist.vendor.mtk_rcs_support=1
else ifeq ($(strip $(MTK_RCS_SUPPORT)),no)
  CIP_PROPERTY_OVERRIDES += persist.vendor.mtk_rcs_support=0
endif

# RTT support
ifeq ($(strip $(MTK_RTT_SUPPORT)),yes)
  CIP_PROPERTY_OVERRIDES += persist.vendor.mtk_rtt_support=1
else ifeq ($(strip $(MTK_RTT_SUPPORT)),no)
  CIP_PROPERTY_OVERRIDES += persist.vendor.mtk_rtt_support=0
endif

# VzW Device Type
ifeq ($(strip $(MSSI_MTK_VZW_DEVICE_TYPE)), 0)
  CIP_PROPERTY_OVERRIDES += persist.vendor.vzw_device_type=0
endif
ifeq ($(strip $(MSSI_MTK_VZW_DEVICE_TYPE)), 1)
  CIP_PROPERTY_OVERRIDES += persist.vendor.vzw_device_type=1
endif
ifeq ($(strip $(MSSI_MTK_VZW_DEVICE_TYPE)), 2)
  CIP_PROPERTY_OVERRIDES += persist.vendor.vzw_device_type=2
endif
ifeq ($(strip $(MSSI_MTK_VZW_DEVICE_TYPE)), 3)
  CIP_PROPERTY_OVERRIDES += persist.vendor.vzw_device_type=3
endif
ifeq ($(strip $(MSSI_MTK_VZW_DEVICE_TYPE)), 4)
  CIP_PROPERTY_OVERRIDES += persist.vendor.vzw_device_type=4
endif
