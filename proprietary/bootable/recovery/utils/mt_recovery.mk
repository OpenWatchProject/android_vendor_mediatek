#
# Copyright (C) 2014 MediaTek Inc.
# Modification based on code covered by the mentioned copyright
# and/or permission notice(s).
#
MEDIATEK_RECOVERY_PATH := vendor/mediatek/proprietary/bootable/recovery

LOCAL_C_INCLUDES += \
                   $(MEDIATEK_RECOVERY_PATH)/utils/include

ifeq ($(MTK_AEE_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_AEE_SUPPORT=1
endif

ifeq ($(MTK_GMO_ROM_OPTIMIZE),true)
LOCAL_CFLAGS += -DMTK_GMO_ROM_OPTIMIZE
endif

ifeq ($(TARGET_USERIMAGES_USE_UBIFS),true)
LOCAL_CFLAGS += -DUBIFS_SUPPORT

LOCAL_STATIC_LIBRARIES += ubi_ota_update

endif

##########################################
# Static library - UBIFS_SUPPORT
##########################################

ifeq ($(TARGET_USERIMAGES_USE_UBIFS),true)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := roots.cpp \
                   ../../$(MEDIATEK_RECOVERY_PATH)/mt_roots.cpp

LOCAL_MODULE := ubiutils

LOCAL_C_INCLUDES += system/extras/ext4_utils \
                    $(MEDIATEK_RECOVERY_PATH) \
                    system/core/fs_mgr/include \
                    system/core/fs_mgr \
                    $(MEDIATEK_RECOVERY_PATH)/utils/include

LOCAL_STATIC_LIBRARIES += libz ubi_ota_update

LOCAL_CFLAGS += -DUBIFS_SUPPORT

##########################################
# Add for FAT merge
##########################################
ifeq ($(MTK_MLC_NAND_SUPPORT),yes)
LOCAL_CFLAGS += -DBOARD_UBIFS_FAT_MERGE_VOLUME_SIZE=$(BOARD_UBIFS_FAT_MERGE_VOLUME_SIZE)
LOCAL_CFLAGS += -DBOARD_UBIFS_IPOH_VOLUME_SIZE=$(BOARD_UBIFS_IPOH_VOLUME_SIZE)
endif

LOCAL_MODULE_TAGS := eng

include $(BUILD_STATIC_LIBRARY)
endif
