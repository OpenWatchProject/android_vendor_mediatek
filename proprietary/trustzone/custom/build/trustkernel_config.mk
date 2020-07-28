include $(TRUSTZONE_CUSTOM_BUILD_PATH)/common_config.mk

##
# global setting
##
TEE_BUILD_MODE ?= Debug Release
ifeq ($(TARGET_BUILD_VARIANT),eng)
  TEE_INSTALL_MODE ?= Debug
else
  TEE_INSTALL_MODE ?= Release
endif

ifeq ($(TEE_INSTALL_MODE),Debug)
  TEE_INSTALL_MODE_LC := debug
else
  TEE_INSTALL_MODE_LC := release
endif

### TEE SETTING ###

TEE_ADDITIONAL_DEPENDENCIES := $(abspath $(TRUSTZONE_PROJECT_MAKEFILE) $(TRUSTZONE_CUSTOM_BUILD_PATH)/common_config.mk $(TRUSTZONE_CUSTOM_BUILD_PATH)/trustkernel_config.mk)
TEE_PACKAGE_PATH := vendor/mediatek/proprietary/trustzone/trustkernel/source
ifneq ($(MTK_MACH_TYPE),)
TEE_RAW_IMAGE_NAME := $(TEE_PACKAGE_PATH)/bsp/platform/$(MTK_MACH_TYPE)/tee/tee.bin
else
TEE_RAW_IMAGE_NAME := $(TEE_PACKAGE_PATH)/bsp/platform/$(ARCH_MTK_PLATFORM)/tee/tee.bin
endif
TEE_TEMP_IMM_IMAGE_NAME := $(TRUSTZONE_IMAGE_OUTPUT_PATH)/bin/$(ARCH_MTK_PLATFORM)_$(MTK_TARGET_PROJECT)_tee_$(TEE_INSTALL_MODE_LC).injected.bin
TEE_TEMP_PADDING_FILE := $(TRUSTZONE_IMAGE_OUTPUT_PATH)/bin/$(ARCH_MTK_PLATFORM)_tee_$(TEE_INSTALL_MODE_LC)_pad.txt
TEE_TEMP_CFG_FILE := $(TRUSTZONE_IMAGE_OUTPUT_PATH)/bin/img_hdr_tee.cfg
TEE_SIGNED_IMAGE_NAME := $(TRUSTZONE_IMAGE_OUTPUT_PATH)/bin/$(ARCH_MTK_PLATFORM)_tee_$(TEE_INSTALL_MODE_LC)_signed.img
TEE_PADDING_IMAGE_NAME := $(TRUSTZONE_IMAGE_OUTPUT_PATH)/bin/$(ARCH_MTK_PLATFORM)_tee_$(TEE_INSTALL_MODE_LC)_pad.img
TEE_COMP_IMAGE_NAME := $(TRUSTZONE_IMAGE_OUTPUT_PATH)/bin/$(ARCH_MTK_PLATFORM)_tee.img

$(TEE_TEMP_PADDING_FILE): ALIGNMENT=512
$(TEE_TEMP_PADDING_FILE): MKIMAGE_HDR_SIZE=512
$(TEE_TEMP_PADDING_FILE): RSA_SIGN_HDR_SIZE=576
$(TEE_TEMP_PADDING_FILE): $(TEE_RAW_IMAGE_NAME) $(TEE_ADDITIONAL_DEPENDENCIES)
	@echo TEE build: $@
	$(hide) mkdir -p $(dir $@)
	$(hide) rm -f $@
	$(hide) FILE_SIZE=$$(($$(wc -c < "$(TEE_RAW_IMAGE_NAME)")+$(MKIMAGE_HDR_SIZE)+$(RSA_SIGN_HDR_SIZE)));\
	REMAINDER=$$(($${FILE_SIZE} % $(ALIGNMENT)));\
	if [ $${REMAINDER} -ne 0 ]; then dd if=/dev/zero of=$@ bs=$$(($(ALIGNMENT)-$${REMAINDER})) count=1; else touch $@; fi

$(TEE_TEMP_CFG_FILE): $(TEE_DRAM_SIZE_CFG) $(TEE_ADDITIONAL_DEPENDENCIES)
	@echo TEE build: $@
	$(hide) mkdir -p $(dir $@)
	$(hide) rm -f $@
	@echo "LOAD_MODE = 0" > $@
	@echo "NAME = tee" >> $@
	@echo "LOAD_ADDR =" $(TEE_TOTAL_DRAM_SIZE) >> $@

$(TEE_PADDING_IMAGE_NAME): $(TEE_RAW_IMAGE_NAME) $(TEE_TEMP_PADDING_FILE) $(TEE_ADDITIONAL_DEPENDENCIES)
	@echo TEE build: $@
	$(hide) mkdir -p $(dir $@)
	$(hide) $(TEE_PACKAGE_PATH)/tools/inject_project_cert.py --cert $(TEE_PACKAGE_PATH)/build/volume.dat --in $(TEE_RAW_IMAGE_NAME) --out $(TEE_TEMP_IMM_IMAGE_NAME)
	$(hide) cat $(TEE_TEMP_IMM_IMAGE_NAME) $(TEE_TEMP_PADDING_FILE) > $@

$(TEE_SIGNED_IMAGE_NAME): ALIGNMENT=512
$(TEE_SIGNED_IMAGE_NAME): $(TEE_PADDING_IMAGE_NAME) $(TRUSTZONE_SIGN_TOOL) $(TRUSTZONE_IMG_PROTECT_CFG) $(TEE_DRAM_SIZE_CFG) $(TEE_ADDITIONAL_DEPENDENCIES)
	@echo TEE build: $@
	$(hide) mkdir -p $(dir $@)
	$(hide) $(TRUSTZONE_SIGN_TOOL) $(TRUSTZONE_IMG_PROTECT_CFG) $(TEE_PADDING_IMAGE_NAME) $@ $(TEE_DRAM_SIZE)
	$(hide) FILE_SIZE=$$(wc -c < "$(TEE_SIGNED_IMAGE_NAME)");REMAINDER=$$(($${FILE_SIZE} % $(ALIGNMENT)));\
	if [ $${REMAINDER} -ne 0 ]; then echo "[ERROR] File $@ size $${FILE_SIZE} is not $(ALIGNMENT) bytes aligned";exit 1; fi

$(TEE_COMP_IMAGE_NAME): ALIGNMENT=512
$(TEE_COMP_IMAGE_NAME): $(TEE_SIGNED_IMAGE_NAME) $(MTK_MKIMAGE_TOOL) $(TEE_TEMP_CFG_FILE) $(TEE_ADDITIONAL_DEPENDENCIES)
	@echo TEE build: $@
	$(hide) mkdir -p $(dir $@)
	$(hide) $(MTK_MKIMAGE_TOOL) $(TEE_SIGNED_IMAGE_NAME) $(TEE_TEMP_CFG_FILE) > $@
	$(hide) FILE_SIZE=$$(stat -c%s "$(TEE_COMP_IMAGE_NAME)");REMAINDER=$$(($${FILE_SIZE} % $(ALIGNMENT)));\
	if [ $${REMAINDER} -ne 0 ]; then echo "[ERROR] File $@ size $${FILE_SIZE} is not $(ALIGNMENT) bytes aligned";exit 1; fi
