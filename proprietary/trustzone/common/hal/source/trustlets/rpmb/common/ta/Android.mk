ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# OUTPUT_NAME
LOCAL_MODULE := tlrpmb_gp

# TBASE_API_LEVEL
LOCAL_CFLAGS += -DTBASE_API_LEVEL=5

#
# Attention:
# Setting CFG_RPMB_KEY_PROGRAMED_IN_KERNEL as 1 means accepting the risk of revealing key in
# Linux Kernel. Mediatek won’t take the responsibility for loss incurred by the key revealing.
#
CFG_RPMB_KEY_PROGRAMED_IN_KERNEL := 0
LOCAL_CFLAGS += -DCFG_RPMB_KEY_PROGRAMED_IN_KERNEL=$(CFG_RPMB_KEY_PROGRAMED_IN_KERNEL)

# SRC_LIB_C
LOCAL_SRC_FILES += \
    src/tlrpmb_gp.c \

# INCLUDE_DIRS
LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/inc \
    $(LOCAL_PATH)/public \

LOCAL_STATIC_LIBRARIES += drrpmb_gp_api

include vendor/mediatek/proprietary/trustzone/custom/build/tee_static_library.mk

endif # MTK_TEE_GP_SUPPORT = yes
