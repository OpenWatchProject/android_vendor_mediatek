ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6757 mt6797 mt6799 mt6759 mt6763 mt6758 mt3886 elbrus mt6775 mt6771 mt6765 mt3967 mt6785))
IS_LEGACY = 0
else
IS_LEGACY = 1
endif

MTK_MTKCAM_PLATFORM := $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)

ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),mt6753))
MTK_MTKCAM_PLATFORM    := $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/mt6735
endif

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6735m mt6737m))
MTK_MTKCAM_PLATFORM    := $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/mt6735m
endif

ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),mt8173))
MTK_MTKCAM_PLATFORM    := $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/mt8173
endif
