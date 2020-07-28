LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    PplAgent.cpp

LOCAL_C_INCLUDES += \
    vendor/mediatek/proprietary/external/nvram/libfile_op \
    system/core/fs_mgr/include_fstab/fstab

# below chips are before MT6755 & MT6797 and have no write protection
ifeq (,$(filter MT6572 MT6580 MT6582 MT6592 MT6735 MT6752 MT6795,$(MTK_PLATFORM)))
    LOCAL_CFLAGS += -DMTK_WRITE_ON_PROTECTION
endif

ifneq (,$(filter MT6755,$(MTK_PLATFORM)))
    LOCAL_CFLAGS += -DRAW_DATA_PARTITION
endif

LOCAL_STATIC_LIBRARIES += libfstab

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    liblog \
    libhwbinder \
    libbase \
    libhidlbase \
    libhidltransport \
    libfile_op \
    vendor.mediatek.hardware.pplagent@1.0

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:= ppl_agent
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_INIT_RC := ppl_agent.rc

include $(MTK_EXECUTABLE)
