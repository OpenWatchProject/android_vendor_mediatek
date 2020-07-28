LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk

LOCAL_SRC_FILES := \
	src/main.cc \
	src/adler32memcpy.cc \
	src/disk_blocks.cc \
	src/error_diag.cc \
	src/finelock_queue.cc \
	src/logger.cc \
	src/os.cc \
	src/os_factory.cc \
	src/pattern.cc \
	src/queue.cc \
	src/sat.cc \
	src/sat_factory.cc \
	src/worker.cc

# Build a 64b version
#LOCAL_MULTILIB := 32
LOCAL_MODULE := doeapp-sat
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_UNSUPPORTED_TARGET_ARCH := mips x86

LOCAL_CFLAGS := -DHAVE_CONFIG_H -DANDROID -DNDEBUG -UDEBUG -DCHECKOPTS
LOCAL_CFLAGS += -Wall -Werror -Wno-unused-parameter -Wno-\#warnings -Wno-implicit-fallthrough

LOCAL_CPP_EXTENSION := .cc

ifeq ($(HAVE_AEE_FEATURE), yes)
	LOCAL_SHARED_LIBRARIES += libaedv
	LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
	LOCAL_C_INCLUDES += $(MTK_ROOT)/external/aee/binary/inc
endif

include $(MTK_EXECUTABLE)
