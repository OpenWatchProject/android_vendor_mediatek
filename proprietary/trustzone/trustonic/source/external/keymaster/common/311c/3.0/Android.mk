# =============================================================================
#
# Module: libMcTeeKeymaster.so - Client library handling key operations
# with TEE Keymaster Trusted Application
#
# =============================================================================

ifndef TRUSTONIC_ANDROID_LEGACY_SUPPORT

LOCAL_PATH := $(call my-dir)

# =============================================================================

include $(CLEAR_VARS)
LOCAL_MODULE := libMcTeeKeymaster

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_CPPFLAGS := -Wall
LOCAL_CPPFLAGS += -Wextra
LOCAL_CPPFLAGS += -Werror
LOCAL_CPPFLAGS += -std=c++11

LOCAL_CPPFLAGS += -DKEYMASTER_WANTED_VERSION=3

# Store attestation key in RPMB or persist partition
ifeq ($(KEYMASTER_RPMB),yes)
LOCAL_CPPFLAGS += -DRPMB_SUPPORT=1
else
LOCAL_CPPFLAGS += -DRPMB_SUPPORT=0
endif

ALL_SRC_FILES := $(wildcard ${LOCAL_PATH}/src/*.cpp \
                            ${LOCAL_PATH}/src/*.c)
LOCAL_SRC_FILES := $(ALL_SRC_FILES:$(LOCAL_PATH)/%=%)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_SHARED_LIBRARIES := libMcClient
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libcrypto

LOCAL_HEADER_LIBRARIES := libhardware_headers

LOCAL_POST_INSTALL_CMD = \
    $(hide) ln -sf $(notdir $(LOCAL_INSTALLED_MODULE)) $(dir $(LOCAL_INSTALLED_MODULE))keystore.$(TARGET_DEVICE).so ; \
            ln -sf $(notdir $(LOCAL_INSTALLED_MODULE)) $(dir $(LOCAL_INSTALLED_MODULE))keystore.$(TARGET_BOARD_PLATFORM).so

include $(BUILD_SHARED_LIBRARY)

# =============================================================================
#
# Module: testTeeKeymaster - test application for libMcTeeKeymaster.so
#
# =============================================================================

include $(CLEAR_VARS)
LOCAL_MODULE := testTeeKeymaster
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_CPPFLAGS := -Wall
LOCAL_CPPFLAGS += -Wextra
LOCAL_CPPFLAGS += -Werror
LOCAL_CPPFLAGS += -D__STDC_LIMIT_MACROS
LOCAL_CPPFLAGS += -std=c++11

LOCAL_CPPFLAGS += -DKEYMASTER_WANTED_VERSION=3
LOCAL_CPPFLAGS += -DLOG_TAG=\"testTeeKeymaster3\"

ALL_TST_FILES := $(wildcard ${LOCAL_PATH}/test/*.cpp)
LOCAL_SRC_FILES := $(ALL_TST_FILES:$(LOCAL_PATH)/%=%)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/test

LOCAL_SHARED_LIBRARIES := libMcTeeKeymaster libMcClient
ifeq ($(APP_PROJECT_PATH),)
LOCAL_SHARED_LIBRARIES += liblog
else
LOCAL_C_INCLUDES += \
	$(COMP_PATH_AndroidIncludes)

LOCAL_LDLIBS := -llog
endif
LOCAL_SHARED_LIBRARIES += libcrypto

LOCAL_HEADER_LIBRARIES := libhardware_headers

include $(BUILD_EXECUTABLE)

# =============================================================================
#
# Module: gtestTeeKeymaster - test application for libMcTeeKeymaster.so
#
# =============================================================================
ifneq ($(APP_PROJECT_PATH),)
include $(CLEAR_VARS)
LOCAL_MODULE := gtestTeeKeymaster
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_CPPFLAGS := -Wall
LOCAL_CPPFLAGS += -Werror=missing-field-initializers
LOCAL_CPPFLAGS += -Werror
LOCAL_CPPFLAGS += -D__STDC_LIMIT_MACROS
LOCAL_CPPFLAGS += -std=gnu++11

LOCAL_CPPFLAGS += -DKEYMASTER_WANTED_VERSION=3
LOCAL_CPPFLAGS += -DLOG_TAG=\"testTeeKeymaster3\"

ALL_TST_FILES := $(wildcard ${LOCAL_PATH}/test/gtest/*.cpp)
LOCAL_SRC_FILES := $(ALL_TST_FILES:$(LOCAL_PATH)/%=%)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include $(LOCAL_PATH)/test/gtest
LOCAL_C_INCLUDES += $(ANDROID_PACK_PATH_TlcTeeGatekeeper)/include
LOCAL_C_INCLUDES += $(ANDROID_PACK_PATH_TlcTeeGatekeeper)/include/google
LOCAL_SHARED_LIBRARIES := libMcGatekeeper libMcTeeKeymaster libMcClient
LOCAL_STATIC_LIBRARIES := libgtest_ndk libgtest_main_ndk
ifeq ($(APP_PROJECT_PATH),)
LOCAL_SHARED_LIBRARIES += liblog
else
LOCAL_C_INCLUDES += \
	$(COMP_PATH_AndroidIncludes)

LOCAL_LDLIBS := -llog
endif
LOCAL_SHARED_LIBRARIES += libcrypto

LOCAL_HEADER_LIBRARIES := libhardware_headers

include $(BUILD_EXECUTABLE)
endif

$(call import-add-path,/)
$(call import-module,$(COMP_PATH_AndroidBoringSsl))
$(call import-module,$(COMP_PATH_MobiCoreClientLib_module))
$(call import-module,$(COMP_PATH_AndroidGTest))
$(call import-module,$(ANDROID_PACK_PATH_TlcTeeGatekeeper))

endif # !TRUSTONIC_ANDROID_LEGACY_SUPPORT
