AUDIO_PARAM_OUT_DIR := $(TARGET_OUT_VENDOR_ETC)/audio_param
EXTRACT_FILE_LIST := *_AudioParam.xml *_ParamUnitDesc.xml *_ParamTreeView.xml

LOCAL_AUDIO_PARAM_FILE_PATTERN := $(subst *,%,$(EXTRACT_FILE_LIST))
LOCAL_AUDIO_PARAM_INSTALLED :=

ifneq (,$(strip $(INSTALL_AUDIO_PARAM_FILE_LIST))$(strip $(INSTALL_AUDIO_PARAM_DIR_LIST)))
  $(warning $(newline)\
*********************************************************************$(newline)\
INSTALL_AUDIO_PARAM_FILE_LIST and INSTALL_AUDIO_PARAM_DIR_LIST are no$(newline)\
longer supported, but observed the following usages:$(newline)$(newline)\
$(if $(strip $(INSTALL_AUDIO_PARAM_FILE_LIST)),$(space)$(space)INSTALL_AUDIO_PARAM_FILE_LIST: $(INSTALL_AUDIO_PARAM_FILE_LIST)$(newline))\
$(if $(strip $(INSTALL_AUDIO_PARAM_DIR_LIST)),$(space)$(space)INSTALL_AUDIO_PARAM_DIR_LIST: $(INSTALL_AUDIO_PARAM_DIR_LIST)$(newline))\
$(newline)\
To add custom AudioParam files, please use:$(newline)\
$(space)$(space)CUSTOM_AUDIO_PARAM_FILE_LIST += $$(YOUR_FILES)$(newline)\
Likewise, for custom AudioParam directories:$(newline)\
$(space)$(space)CUSTOM_AUDIO_PARAM_DIR_LIST += $$(YOUR_DIRS)$(newline)\
*********************************************************************)
  $(error Please use CUSTOM_AUDIO_PARAM_FILE_LIST and CUSTOM_AUDIO_PARAM_DIR_LIST instead of INSTALL_AUDIO_PARAM_FILE_LIST and INSTALL_AUDIO_PARAM_DIR_LIST)
endif

# Add chip & project's default.audio_param by default
CHIP := $(MTK_PLATFORM_DIR)
CUSTOM_AUDIO_PARAM_FILE_LIST += $(MTK_TARGET_PROJECT_FOLDER)/default.audio_param device/mediatek/$(CHIP)/default.audio_param

# Deploy these files in MTK_AUDIO_PARAM_FILE_LIST to LOCAL_DEFAULT_AUDIO_PARAM_FILE
# Check if the audio_param exist, uncompress & delete it
LOCAL_DEFAULT_AUDIO_PARAM_FILE := $(firstword $(wildcard $(CUSTOM_AUDIO_PARAM_FILE_LIST) $(MTK_AUDIO_PARAM_FILE_LIST)))

ifneq (,$(filter custom_images,$(MAKECMDGOALS)))
AUDIO_PARAM_CUSTOM_IMAGE_ZIP_FILE := $(LOCAL_DEFAULT_AUDIO_PARAM_FILE)
AUDIO_PARAM_CUSTOM_IMAGE_XML_FILES :=
AUDIO_PARAM_CUSTOM_IMAGE_ZIPPED_FILES :=
endif

ifneq ($(LOCAL_DEFAULT_AUDIO_PARAM_FILE),)
LOCAL_AUDIO_PARAM_UNZIP_FILE_LIST := $(filter $(LOCAL_AUDIO_PARAM_FILE_PATTERN),$(shell unzip -Z -1 $(LOCAL_DEFAULT_AUDIO_PARAM_FILE)))

# $(1): input.zip
# $(2): output.file
define unzip-audio-param-file
$(2): $(1)
	mkdir -p $$(dir $(2))
	unzip -qo $(1) $$(notdir $(2)) -d $$(dir $(2))
endef
$(foreach f,$(LOCAL_AUDIO_PARAM_UNZIP_FILE_LIST),\
	$(eval src := $(LOCAL_DEFAULT_AUDIO_PARAM_FILE))\
	$(eval dst := $(AUDIO_PARAM_OUT_DIR)/$(notdir $(f)))\
	$(if $(filter custom_images,$(MAKECMDGOALS)), \
		$(eval AUDIO_PARAM_CUSTOM_IMAGE_ZIPPED_FILES += $(src)))\
	$(eval LOCAL_AUDIO_PARAM_INSTALLED += $(dst))\
	$(eval $(call unzip-audio-param-file,$(src),$(dst)))\
)
endif

LOCAL_AUDIO_PARAM_COPY_FILE_LIST := $(filter $(LOCAL_AUDIO_PARAM_FILE_PATTERN),$(foreach d,$(CUSTOM_AUDIO_PARAM_DIR_LIST) $(MTK_AUDIO_PARAM_DIR_LIST),$(wildcard $(d)/*.xml)))
LOCAL_AUDIO_PARAM_COPY_FILE_STEM := $(sort $(filter-out $(notdir $(LOCAL_AUDIO_PARAM_UNZIP_FILE_LIST)),$(notdir $(LOCAL_AUDIO_PARAM_COPY_FILE_LIST))))
$(foreach f,$(LOCAL_AUDIO_PARAM_COPY_FILE_STEM),\
	$(eval chk := $(filter %/$(f),$(LOCAL_AUDIO_PARAM_COPY_FILE_LIST)))\
	$(eval src := $(firstword $(chk)))\
	$(if $(filter custom_images,$(MAKECMDGOALS)),\
		$(eval AUDIO_PARAM_CUSTOM_IMAGE_XML_FILES += $(src)))\
	$(eval exc := $(filter-out $(src),$(chk)))\
	$(if $(strip $(exc)),$(info AudioParam: $(src) overrides $(exc)))\
	$(eval dst := $(AUDIO_PARAM_OUT_DIR)/$(f))\
	$(eval LOCAL_AUDIO_PARAM_INSTALLED += $(dst))\
	$(eval $(call copy-one-file,$(src),$(dst)))\
)

ifneq (,$(filter custom_images,$(MAKECMDGOALS)))
AUDIO_PARAM_CUSTOM_IMAGE_ZIP_FILE := \
	$(strip $(AUDIO_PARAM_CUSTOM_IMAGE_ZIP_FILE))
AUDIO_PARAM_CUSTOM_IMAGE_ZIPPED_FILES := \
	$(strip $(AUDIO_PARAM_CUSTOM_IMAGE_ZIPPED_FILES))
AUDIO_PARAM_CUSTOM_IMAGE_XML_FILES := \
	$(strip $(AUDIO_PARAM_CUSTOM_IMAGE_XML_FILES))
endif

ALL_DEFAULT_INSTALLED_MODULES += $(LOCAL_AUDIO_PARAM_INSTALLED)
.PHONY: MODULES-IN-vendor-mediatek-proprietary-external-AudioParamParser
MODULES-IN-vendor-mediatek-proprietary-external-AudioParamParser: $(LOCAL_AUDIO_PARAM_INSTALLED)
