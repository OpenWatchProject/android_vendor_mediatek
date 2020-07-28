###############################################################
# build dummy tcon
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(MTK_HWC_SUPPORT), yes)
ifeq ($(MTK_HWC_VERSION), 1.5.0)

LOCAL_SRC_FILES := \
	tcon_shell.cpp \
	tcon_impl.cpp

LOCAL_CFLAGS := \
	-DLOG_TAG=\"dummy_tcon\"

LOCAL_C_INCLUDES := \
	$(TOP)/$(MTK_ROOT)/hardware/hwcomposer/include

LOCAL_MODULE := libtcon_dummy

include $(BUILD_SHARED_LIBRARY)

endif
endif
