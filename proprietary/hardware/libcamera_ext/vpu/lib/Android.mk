LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

vpu_binary_folder := $(LOCAL_PATH)/$(MTK_PLATFORM_DIR)
nn_binary_folder := $(MTK_PATH_SOURCE)/hardware/vpu_nn_algo/$(MTK_PLATFORM_DIR)

ifneq (,$(wildcard $(vpu_binary_folder)/main_imggen))

LOCAL_MODULE := cam_vpu1.img
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_PATH := $(PRODUCT_OUT)
include $(BUILD_SYSTEM)/base_rules.mk

vpu_binaries := $(shell find $(vpu_binary_folder)/main -type f)

$(LOCAL_BUILT_MODULE): PRIVATE_VPU_BINARY_FOLDER := $(vpu_binary_folder)
$(LOCAL_BUILT_MODULE): $(vpu_binaries) $(vpu_binary_folder)/main_imggen
	@echo Pack vpu binaries: $@
	$(hide) mkdir -p $(dir $@)
	$(hide) $(PRIVATE_VPU_BINARY_FOLDER)/main_imggen -i $(PRIVATE_VPU_BINARY_FOLDER)/main/ -o $@ -m 1

include $(CLEAR_VARS)

LOCAL_MODULE := cam_vpu2.img
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_PATH := $(PRODUCT_OUT)
include $(BUILD_SYSTEM)/base_rules.mk

vpu_binaries := $(shell find $(vpu_binary_folder)/core* -type f)
ifneq ($(wildcard $(nn_binary_folder)/core*),)
nn_binaries := $(shell find $(nn_binary_folder)/core* -type f)
endif

$(LOCAL_BUILT_MODULE): PRIVATE_VPU_BINARY_FOLDER := $(vpu_binary_folder)
$(LOCAL_BUILT_MODULE): PRIVATE_NN_BINARY_FOLDER := $(nn_binary_folder)
$(LOCAL_BUILT_MODULE): $(vpu_binaries) $(nn_binaries) $(vpu_binary_folder)/main_imggen
	@echo Pack vpu binaries: $@
	$(hide) mkdir -p $(dir $@)
ifeq ($(wildcard $(nn_binary_folder)),)
	$(hide) $(PRIVATE_VPU_BINARY_FOLDER)/main_imggen -i $(PRIVATE_VPU_BINARY_FOLDER)/ -o $@ -m 2
else
	$(hide) $(PRIVATE_VPU_BINARY_FOLDER)/main_imggen -i $(PRIVATE_VPU_BINARY_FOLDER)/ -j $(PRIVATE_NN_BINARY_FOLDER)/ -o $@ -m 2
endif
include $(CLEAR_VARS)

LOCAL_MODULE := cam_vpu3.img
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_PATH := $(PRODUCT_OUT)
include $(BUILD_SYSTEM)/base_rules.mk

vpu_binaries := $(shell find $(vpu_binary_folder)/custom -type f)

$(LOCAL_BUILT_MODULE): PRIVATE_VPU_BINARY_FOLDER := $(vpu_binary_folder)
$(LOCAL_BUILT_MODULE): $(vpu_binaries) $(vpu_binary_folder)/main_imggen
	@echo Pack vpu binaries: $@
	$(hide) mkdir -p $(dir $@)
	$(hide) $(PRIVATE_VPU_BINARY_FOLDER)/main_imggen -i $(PRIVATE_VPU_BINARY_FOLDER)/custom/ -o $@ -m 3

endif
