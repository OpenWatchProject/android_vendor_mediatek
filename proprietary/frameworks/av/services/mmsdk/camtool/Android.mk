ifneq ($(strip $(TARGET_BUILD_VARIANT)),user)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#-----------------------------------------------------------
LOCAL_SRC_FILES += camtool.cpp

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

ifneq ($(filter $(TARGET_PRODUCT),full_tb8765ap1_bsp_1g), )
LOCAL_CFLAGS += -DMTKCAM_CCT_TUNING_FOR_LOWDRAM
endif

LOCAL_SHARED_LIBRARIES += \
    liblog \
    libcutils \
    libutils

LOCAL_WHOLE_STATIC_LIBRARIES +=

LOCAL_STATIC_LIBRARIES +=

#-----------------------------------------------------------
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE := camtool
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := false

#-----------------------------------------------------------
include $(MTK_EXECUTABLE)
endif
