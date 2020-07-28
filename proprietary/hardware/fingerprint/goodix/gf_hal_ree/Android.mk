#
# Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
# All Rights Reserved.
#

LOCAL_PATH := $(call my-dir)

#auto modify it in script(release_for_xxxx.sh)
FACTORY_TEST:=ree

include $(CLEAR_VARS)

ifeq ($(FACTORY_TEST), ree)
    include $(CLEAR_VARS)
    LOCAL_MODULE_TAGS := optional
    LOCAL_MODULE := libgf_hal_ree
    LOCAL_PROPRIETARY_MODULE := true
    LOCAL_MODULE_OWNER := mtk

    ifeq ($(TARGET_BUILD_VARIANT),eng)
	    LOCAL_SRC_FILES_64 := debug/arm64-v8a/libgf_hal_ree.so
    else ifeq ($(TARGET_BUILD_VARIANT),userdebug)
	    LOCAL_SRC_FILES_64 := userdebug/arm64-v8a/libgf_hal_ree.so
    else
	    LOCAL_SRC_FILES_64 := release/arm64-v8a/libgf_hal_ree.so
    endif

    LOCAL_SHARED_LIBRARIES := \
        libcutils \
        libgf_ca_ree \
        libgf_ta_ree

    LOCAL_MULTILIB := 64
    LOCAL_MODULE_CLASS := SHARED_LIBRARIES
    LOCAL_MODULE_SUFFIX := .so
    include $(BUILD_PREBUILT)
endif

ifneq ($(FACTORY_TEST), none)
    include $(CLEAR_VARS)

    # Module name (sets name of output binary / library)
    LOCAL_MODULE    := libgf_hal_factory
    LOCAL_PROPRIETARY_MODULE := true
    LOCAL_MODULE_OWNER := mtk

    LOCAL_C_INCLUDES := \
        $(LOCAL_PATH)/public \
        $(LOCAL_PATH)/include \
        $(LOCAL_PATH)/factory_test

    LOCAL_SRC_FILES+= factory_test/gf_hal_factory_test.c \
               factory_test/ftm_test_result_checker.c \
               factory_test/ftm_test_result_parser.c

    LOCAL_SHARED_LIBRARIES := \
        liblog \
        libutils \
        libgf_hal_ree \
        libgf_ca_ree \
        libgf_ta_ree


    #LOCAL_CFLAGS += -Werror

    # GF_LOG_LEVEL is 3, will output debug, info, error log
    # GF_LOG_LEVEL is 2, will output info, error log
    # GF_LOG_LEVEL is 1, only output error log
    ifeq ($(TARGET_MODE), debug)
    LOCAL_CFLAGS += -DGF_LOG_LEVEL=3
    else
    LOCAL_CFLAGS += -DGF_LOG_LEVEL=2
    endif

    ifeq ($(FACTORY_TEST), ree)
    LOCAL_CFLAGS += -DFACTORY_TEST_REE
    endif

    LOCAL_CONLYFLAGS += -std=c99
    LOCAL_CPPFLAGS += -std=c++0x

    LOCAL_LDLIBS := -llog
    LOCAL_MODULE_TAGS := optional

    include $(BUILD_SHARED_LIBRARY)
endif


