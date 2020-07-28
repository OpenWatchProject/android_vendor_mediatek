LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := build_keybox_crypt
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_OWNER := trustkernel
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
	build_keybox_crypt.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../../../build

include $(BUILD_HOST_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE := 6B6579626F785F6372797074
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := trustkernel
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_CLASS := FAKE
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/app/t6/data/E62563B4905C52822EADA8E32E5180D6

FS_TOOL := $(LOCAL_PATH)/../../../tools/build_ro_po

include $(BUILD_SYSTEM)/base_rules.mk

$(LOCAL_BUILT_MODULE): $(HOST_OUT_EXECUTABLES)/build_keybox_crypt $(FS_TOOL)
	$(hide) echo "Generating keybox_crypt... $@"
	$(hide) $(HOST_OUT_EXECUTABLES)/build_keybox_crypt $(dir $@)keybox_crypt
	$(hide) echo "Build keybox_crypt as simple obj..."
	$(hide) $(FS_TOOL) $(dir $@)keybox_crypt $@

