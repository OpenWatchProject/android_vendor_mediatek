ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# OUTPUT_NAME
LOCAL_MODULE := drrpmb_gp

# DRIVER INFORMATION
DRIVER_VENDOR_ID := 0
DRIVER_NUMBER := 0x0380
DRIVER_ID := $$(($(DRIVER_VENDOR_ID)<<16|$(DRIVER_NUMBER)))

# DRIVER_ID
LOCAL_CFLAGS += -DDRIVER_ID=$(DRIVER_ID)

# TBASE_API_LEVEL
LOCAL_CFLAGS += -DTBASE_API_LEVEL=5

#
# Attention:
# Setting CFG_RPMB_KEY_PROGRAMED_IN_KERNEL as 1 means accepting the risk of revealing key in
# Linux Kernel. Mediatek won't take the responsibility for loss incurred by the key revealing.
#
CFG_RPMB_KEY_PROGRAMED_IN_KERNEL := 0
LOCAL_CFLAGS += -DCFG_RPMB_KEY_PROGRAMED_IN_KERNEL=$(CFG_RPMB_KEY_PROGRAMED_IN_KERNEL)

# SRC_LIB_C
LOCAL_SRC_FILES += \
    core/rpmb_main.c \
    core/rpmb_ipch.c \
    core/rpmb_ops.c \
    core/rpmb_dcih.c


# INCLUDE_DIRS
LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/inc \
    $(LOCAL_PATH)/public \
    $(LOCAL_PATH)/../../../secmem/common/drv_api_inc \
    vendor/mediatek/proprietary/bootable/bootloader/preloader/platform/$(MTK_PLATFORM_DIR)/src/core/inc \
    vendor/mediatek/proprietary/bootable/bootloader/preloader/platform/$(MTK_PLATFORM_DIR)/src/security/trustzone/inc

include vendor/mediatek/proprietary/trustzone/custom/build/tee_static_library.mk


include $(CLEAR_VARS)

# OUTPUT_NAME
LOCAL_MODULE := drrpmb_gp_api

# DRIVER INFORMATION
DRIVER_VENDOR_ID := 0
DRIVER_NUMBER := 0x0380
DRIVER_ID := $$(($(DRIVER_VENDOR_ID)<<16|$(DRIVER_NUMBER)))

# DRIVER_ID
LOCAL_CFLAGS += -DDRIVER_ID=$(DRIVER_ID)

# TBASE_API_LEVEL
LOCAL_CFLAGS += -DTBASE_API_LEVEL=5

# ARMCC_SHORT_ENUMS
#LOCAL_CFLAGS += -fno-short-enums

#
# Attention:
# Setting CFG_RPMB_KEY_PROGRAMED_IN_KERNEL as 1 means accepting the risk of revealing key in
# Linux Kernel. Mediatek won't take the responsibility for loss incurred by the key revealing.
#
CFG_RPMB_KEY_PROGRAMED_IN_KERNEL := 0
LOCAL_CFLAGS += -DCFG_RPMB_KEY_PROGRAMED_IN_KERNEL=$(CFG_RPMB_KEY_PROGRAMED_IN_KERNEL)

# SRC_LIB_C
LOCAL_SRC_FILES += \
    api/tlRpmbDriverApi.c

# INCLUDE_DIRS
LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/inc \
    $(LOCAL_PATH)/public

LOCAL_EXPORT_C_INCLUDE_DIRS += $(LOCAL_PATH)/public

include vendor/mediatek/proprietary/trustzone/custom/build/tee_static_library.mk

endif # MTK_TEE_GP_SUPPORT = yes
