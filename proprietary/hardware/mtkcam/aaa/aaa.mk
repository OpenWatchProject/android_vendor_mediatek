#-----------------------------------------------------------
# 3A/ISP Co-platform seting
#-----------------------------------------------------------

MTKCAM_IS_IP_BASE := 0
MTKCAM_COMMON_HAL3A_VERSION := v2.0

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6771 mt6775 mt6785))
    MTKCAM_AAA_PLATFORM := isp_50
    MTKCAM_IS_IP_BASE := 1
    MTKCAM_COMMON_HAL3A_VERSION := v1.0
else ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6779))
    MTKCAM_AAA_PLATFORM := isp_60
    MTKCAM_IS_IP_BASE := 1
else ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6883 mt6885))
    MTKCAM_AAA_PLATFORM := isp_6s
    MTKCAM_IS_IP_BASE := 1
    MTKCAM_COMMON_HAL3A_VERSION := v3.0
else ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6739 mt6761))
    MTKCAM_AAA_PLATFORM := isp_30
    MTKCAM_IS_IP_BASE := 1
    MTKCAM_COMMON_HAL3A_VERSION := v1.0
else ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6757 mt6763 mt6765 mt6768))
    ifeq ($(CAMERA_HAL_VERSION), 3)
        # Hal3
        MTKCAM_AAA_PLATFORM := isp_40
        MTKCAM_IS_IP_BASE := 1
        MTKCAM_COMMON_HAL3A_VERSION := v1.0
    else
        # Hal1
        MTKCAM_AAA_PLATFORM := $(TARGET_BOARD_PLATFORM)
    endif
else
    MTKCAM_AAA_PLATFORM := $(TARGET_BOARD_PLATFORM)
endif

# Default Setting
MTKCAM_IS_ISP_30_PLATFORM := 0
MTKCAM_IS_ISP_40_PLATFORM := 0
MTKCAM_IS_ISP_50_PLATFORM := 0
MTKCAM_LSC_PLATFORM := $(MTKCAM_AAA_PLATFORM)
MTKCAM_NVRAM_PLATFORM := $(MTKCAM_AAA_PLATFORM)
MTKCAM_IS_CCU_EN := 1
MTKCAM_IS_LEGACY_ALGO := 0
MTKCAM_IS_ISP_40_PLATFORM_MIX_VER_1 := 0

# ISP 30 Setting
ifeq ($(MTKCAM_AAA_PLATFORM), isp_30)
    MTKCAM_IS_ISP_30_PLATFORM := 1
    MTKCAM_IS_ISP_50_PLATFORM := 1
    MTKCAM_IS_CCU_EN := 0
    MTKCAM_IS_LEGACY_ALGO := 1
endif

# ISP 40 Setting
ifeq ($(MTKCAM_AAA_PLATFORM), isp_40)
    MTKCAM_IS_ISP_40_PLATFORM := 1
    MTKCAM_IS_ISP_50_PLATFORM := 1
    ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6763 mt6765))
        MTKCAM_LSC_PLATFORM := $(TARGET_BOARD_PLATFORM)
        MTKCAM_NVRAM_PLATFORM := $(TARGET_BOARD_PLATFORM)
    endif
    ifeq ($(TARGET_BOARD_PLATFORM), mt6757)
        MTKCAM_IS_CCU_EN := 0
        MTKCAM_IS_ISP_40_PLATFORM_MIX_VER_1 := 1
    endif
endif

# ISP 50 Setting
ifeq ($(MTKCAM_AAA_PLATFORM), isp_50)
    MTKCAM_IS_ISP_50_PLATFORM := 1
endif

#for user_load judgment
ifeq ($(TARGET_BUILD_VARIANT), user)
LOCAL_CFLAGS += -DIS_BUILD_USER=1
else
LOCAL_CFLAGS += -DIS_BUILD_USER=0
endif
