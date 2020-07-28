
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libgf_ca

LOCAL_CFLAGS += -D_POSIX_THREADS
LOCAL_CFLAGS += -DTBASE_API_LEVEL=5
LOCAL_CFLAGS += -DLOG_ANDROID -UNDEBUG

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../public \
    $(PRODUCT_OUT)/trustzone/trustlet/gf_ta/Public \
    $(LOCAL_PATH)/../../../../../../bsp/platform/$(ARCH_MTK_PLATFORM)/t-sdk/TlcSdk/Out/Public/GP

HAVE_UUID_FILE := $(shell test ! -f $(PRODUCT_OUT)/trustzone/trustlet/gf_ta/Public/gf_ta_uuid.h && echo no)
ifeq ($(HAVE_UUID_FILE), no)
$(shell mkdir -p $(PRODUCT_OUT)/trustzone/trustlet/gf_ta/Public)
$(shell cp -f $(LOCAL_PATH)/../gf_ta_uuid.h $(PRODUCT_OUT)/trustzone/trustlet/gf_ta/Public/gf_ta_uuid.h)
endif

LOCAL_SRC_FILES := trustonic/gf_ca_entry.c

LOCAL_LDLIBS := -llog
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := libMcClient

include $(BUILD_SHARED_LIBRARY)

$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreDriverLib))
