# =============================================================================
#
# HAL implementation of Android Gatekeeper
#
# =============================================================================

ifndef TRUSTONIC_ANDROID_LEGACY_SUPPORT

LOCAL_PATH := $(call my-dir)

# =============================================================================
#
# Module: libMcGatekeeper.so - Client library for Android authorization
# framework
#
# =============================================================================
include $(CLEAR_VARS)

LOCAL_MODULE := libMcGatekeeper
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_CFLAGS := -Wall -Werror -Wmissing-field-initializers -Wunused-parameter
# Software implementation is written in C++11
LOCAL_CPPFLAGS := -std=c++11
LOCAL_CPPFLAGS += -fpermissive -Wall -Werror -D__STDC_LIMIT_MACROS
LOCAL_LDLIBS := -llog

ALL_SRC_FILES := $(wildcard ${LOCAL_PATH}/src/*.c ${LOCAL_PATH}/src/*.cpp)
LOCAL_SRC_FILES := $(ALL_SRC_FILES:$(LOCAL_PATH)/%=%)

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include \
    $(MOBICORE_LIB_PATH)

LOCAL_SHARED_LIBRARIES := libMcClient
LOCAL_SHARED_LIBRARIES += libgatekeeper

LOCAL_HEADER_LIBRARIES := libhardware_headers

LOCAL_POST_INSTALL_CMD = \
    $(hide) ln -sf $(notdir $(LOCAL_INSTALLED_MODULE)) $(dir $(LOCAL_INSTALLED_MODULE))gatekeeper.$(TARGET_DEVICE).so ; \
	    ln -sf $(notdir $(LOCAL_INSTALLED_MODULE)) $(dir $(LOCAL_INSTALLED_MODULE))gatekeeper.$(TARGET_BOARD_PLATFORM).so

include $(BUILD_SHARED_LIBRARY)

# =============================================================================
#
# Module: testTeeGatekeeper - test driver for libMcGatekeeper.so
#
# =============================================================================

ifneq ($(APP_PROJECT_PATH),)

include $(CLEAR_VARS)

LOCAL_MODULE := testTeeGatekeeper
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libMcGatekeeper libMcClient
LOCAL_STATIC_LIBRARIES := libgtest_ndk libgtest_main_ndk
LOCAL_LDLIBS := -llog
LOCAL_CFLAGS := -Wall -Werror -Wmissing-field-initializers -Wunused-parameter
LOCAL_CPPFLAGS := -std=c++11 -fpermissive -Wall -Werror -D__STDC_LIMIT_MACROS

ALL_TST_FILES := $(wildcard ${LOCAL_PATH}/test/*.cpp) $(wildcard ${LOCAL_PATH}/test/utils/*.cpp)
LOCAL_SRC_FILES := $(ALL_TST_FILES:$(LOCAL_PATH)/%=%)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(COMP_PATH_AndroidIncludes)

include $(BUILD_EXECUTABLE)

endif

# =============================================================================
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreClientLib_module))
$(call import-module,$(COMP_PATH_AndroidGTest))

endif # !TRUSTONIC_ANDROID_LEGACY_SUPPORT
