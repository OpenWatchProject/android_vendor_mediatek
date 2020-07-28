#
# Ecc List XML File
#

LOCAL_PATH := vendor/mediatek/proprietary/external/EccList

ECC_FILES:=$(wildcard $(LOCAL_PATH)/ecc_list*.xml)

PRODUCT_COPY_FILES += $(foreach file,$(ECC_FILES), $(file):$(TARGET_COPY_OUT_VENDOR)/etc/$(notdir $(file)):mtk)
