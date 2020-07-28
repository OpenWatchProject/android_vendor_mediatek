##########
#  build the test file of api sample
##########
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	main.cpp

LOCAL_C_INCLUDES := \
	$(TOP)/$(MTK_PATH_SOURCE)/hardware/libgem/inc

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libui \
	libgui \
	libfpspolicy_fw \
	libbinder \
	liblog \
	vendor.mediatek.hardware.dfps@1.0

LOCAL_MULTILIB := both
LOCAL_MODULE := test-policy
LOCAL_MODULE_STEM_64:= ${LOCAL_MODULE}_64
LOCAL_MODULE_STEM_32:= ${LOCAL_MODULE}_32

LOCAL_MODULE_TAGS := tests

include $(BUILD_EXECUTABLE)
