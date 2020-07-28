# Build the unit tests
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

LOCAL_MODULE := mflltest_ut
LOCAL_MODULE_TAGS := tests
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

# camera3test_fixtures.cpp add for start camera3 preview
LOCAL_SRC_FILES := \
	main.cpp \

# libhardware,libcamera_metadata, libdl add for start camera3 preview
LOCAL_SHARED_LIBRARIES := \
	$(MFLL_CORE_LIB_NAME) \
	$(MFLL_SHARED_LIBS) \

LOCAL_C_INCLUDES += $(MFLL_INCLUDE_PATH)


include $(BUILD_EXECUTABLE)
