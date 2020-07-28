###########################################################
# TinySys SCP
###########################################################

# use old tinysys structure when TINYSYS_STRUCTURE_VERSION is empty or 1
ifeq ($(strip $(TINYSYS_STRUCTURE_VERSION)),$(filter 1,$(strip $(TINYSYS_STRUCTURE_VERSION))))

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

TINYSYS_SCP := tinysys-scp

ifeq (yes,$(MTK_TINYSYS_SCP_SUPPORT))
TINYSYS_SCP_TARGET_FILE := scp.img
TINYSYS_SCP_CLASS := TINYSYS_OBJ
TINYSYS_SCP_CLEAN_MODULE := clean-$(TINYSYS_SCP)
TINYSYS_SCP_BUILT_INTERMEDIATES := \
	$(call intermediates-dir-for,$(TINYSYS_SCP_CLASS),$(TINYSYS_SCP))

LOCAL_MODULE := $(TINYSYS_SCP)
LOCAL_MODULE_STEM := $(TINYSYS_SCP_TARGET_FILE)
LOCAL_MODULE_CLASS := ETC
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_PATH := $(PRODUCT_OUT)
include $(BUILD_SYSTEM)/base_rules.mk

TINYSYS_SCP_MAKE_DEPENDENCIES := $(shell find $(LOCAL_PATH) -name .git -prune -o -type f | sort)
TINYSYS_SCP_MAKE_DEPENDENCIES := $(filter-out %/.git %/.gitignore %/.gitattributes,$(TINYSYS_SCP_MAKE_DEPENDENCIES))
TINYSYS_SCP_BUILD_CMD := \
	PROJECT=$(MTK_TARGET_PROJECT) \
	O=$(abspath $(TINYSYS_SCP_BUILT_INTERMEDIATES)) \
	$(if $(filter showcommands,$(MAKECMDGOALS)),V=1) \
	BUILD_TYPE=$(if $(filter-out user,$(TARGET_BUILD_VARIANT)),debug,release) \
	-C $(LOCAL_PATH)

# Config header targets
TINYSYS_SCP_CONFIG_HEADER := tinysys-scp-configheader

.PHONY: $(TINYSYS_SCP_CONFIG_HEADER)
$(TINYSYS_SCP_CONFIG_HEADER):
	$(PREBUILT_MAKE_PREFIX)$(MAKE) $(TINYSYS_SCP_BUILD_CMD) configheader

# Main targets
.PHONY: $(TINYSYS_SCP)
.KATI_RESTAT: $(TINYSYS_SCP_BUILT_INTERMEDIATES)/$(TINYSYS_SCP_TARGET_FILE)
$(TINYSYS_SCP_BUILT_INTERMEDIATES)/$(TINYSYS_SCP_TARGET_FILE): $(TINYSYS_SCP_MAKE_DEPENDENCIES) | check-tinysys-config $(TINYSYS_SCP_CHRE_LINK)
	$(hide)mkdir -p $(dir $@)
	$(PREBUILT_MAKE_PREFIX)$(MAKE) $(TINYSYS_SCP_BUILD_CMD) scp

$(LOCAL_BUILT_MODULE): $(TINYSYS_SCP_BUILT_INTERMEDIATES)/$(TINYSYS_SCP_TARGET_FILE)
	$(hide)mkdir -p $(dir $@)
	$(hide)$(ACP) $< $@

#ifneq (yes,$(MTK_REMAKE_SUBMAKE_FLAG))
#$(LOCAL_BUILT_MODULE) $(TINYSYS_SCP_CONFIG_HEADER): $(TINYSYS_SCP_CLEAN_MODULE)
#endif

else
.PHONY: $(TINYSYS_SCP)
$(TINYSYS_SCP):
endif # MTK_TINYSYS_SCP_SUPPORT

###########################################################
# TinySys ADSP
###########################################################
include $(CLEAR_VARS)

TINYSYS_ADSP := tinysys-adsp

ifeq (yes,$(MTK_AUDIODSP_SUPPORT))
TINYSYS_ADSP_TARGET_FILE := audio_dsp.img
TINYSYS_ADSP_CLASS := TINYSYS_OBJ
TINYSYS_ADSP_BUILT_INTERMEDIATES := \
	$(call intermediates-dir-for,$(TINYSYS_ADSP_CLASS),$(TINYSYS_ADSP))

LOCAL_MODULE := $(TINYSYS_ADSP)
LOCAL_MODULE_STEM := $(TINYSYS_ADSP_TARGET_FILE)
LOCAL_MODULE_CLASS := ETC
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_PATH := $(PRODUCT_OUT)
include $(BUILD_SYSTEM)/base_rules.mk

TINYSYS_ADSP_MAKE_DEPENDENCIES := $(shell find $(LOCAL_PATH) -name .git -prune -o -type f | sort)
TINYSYS_ADSP_MAKE_DEPENDENCIES := $(filter-out %/.git %/.gitignore %/.gitattributes,$(TINYSYS_ADSP_MAKE_DEPENDENCIES))
TINYSYS_ADSP_BUILD_CMD := \
	PROJECT=$(MTK_TARGET_PROJECT) \
	O=$(abspath $(TINYSYS_ADSP_BUILT_INTERMEDIATES)) \
	$(if $(filter showcommands,$(MAKECMDGOALS)),V=1) \
	BUILD_TYPE=$(if $(filter-out user,$(TARGET_BUILD_VARIANT)),debug,release) \
	-C $(LOCAL_PATH)

# Config header targets
TINYSYS_ADSP_CONFIG_HEADER := tinysys-adsp-configheader

.PHONY: $(TINYSYS_ADSP_CONFIG_HEADER)
$(TINYSYS_ADSP_CONFIG_HEADER):
	$(PREBUILT_MAKE_PREFIX)$(MAKE) $(TINYSYS_ADSP_BUILD_CMD) configheader

# Main targets
.PHONY: $(TINYSYS_ADSP)
.KATI_RESTAT: $(TINYSYS_ADSP_BUILT_INTERMEDIATES)/$(TINYSYS_ADSP_TARGET_FILE)
$(TINYSYS_ADSP_BUILT_INTERMEDIATES)/$(TINYSYS_ADSP_TARGET_FILE): $(TINYSYS_ADSP_MAKE_DEPENDENCIES) | check-tinysys-config $(TINYSYS_ADSP_CHRE_LINK)
	$(hide)mkdir -p $(dir $@)
	$(PREBUILT_MAKE_PREFIX)$(MAKE) $(TINYSYS_ADSP_BUILD_CMD) adsp

$(LOCAL_BUILT_MODULE): $(TINYSYS_ADSP_BUILT_INTERMEDIATES)/$(TINYSYS_ADSP_TARGET_FILE)
	$(hide)mkdir -p $(dir $@)
	$(hide)$(ACP) $< $@

#ifneq (yes,$(MTK_REMAKE_SUBMAKE_FLAG))
#$(LOCAL_BUILT_MODULE) $(TINYSYS_ADSP_CONFIG_HEADER): $(TINYSYS_CLEAN_MODULE)
#endif

else
.PHONY: $(TINYSYS_ADSP)
$(TINYSYS_ADSP):
endif # MTK_AUDIODSP_SUPPORT

###########################################################
# Collect NOTICE files
###########################################################
ifneq (,$(MTK_TINYSYS_SCP_SUPPORT)$(MTK_AUDIODSP_SUPPORT))
TINYSYS_NOTICE_FILES := \
  $(shell find $(LOCAL_PATH) -type f -name '*NOTICE*.txt' -printf '%P\n')
TINYSYS_NOTICE_FILES_INSTALLED := \
  $(TINYSYS_NOTICE_FILES:%=$(TARGET_OUT_NOTICE_FILES)/src/tinysys/%)

$(TARGET_OUT_INTERMEDIATES)/NOTICE.html: $(TINYSYS_NOTICE_FILES_INSTALLED)

$(TINYSYS_NOTICE_FILES_INSTALLED): \
  $(TARGET_OUT_NOTICE_FILES)/src/tinysys/%: $(LOCAL_PATH)/% | $(ACP)
	@echo Copying: $@
	$(hide) mkdir -p $(dir $@)
	$(hide) $(ACP) $< $@
endif


###########################################################
# TinySys Check config
###########################################################
# Config option consistency check mechanism
ifneq (,$(filter yes,$(MTK_TINYSYS_SCP_SUPPORT) $(MTK_AUDIODSP_SUPPORT)))

TINYSYS_HEADER_FOLDERS :=
ifneq (,$(TINYSYS_SCP_BUILT_INTERMEDIATES))
TINYSYS_HEADER_FOLDERS += $(TINYSYS_SCP_BUILT_INTERMEDIATES)/freertos/source/CM4_*
endif
ifneq (,$(TINYSYS_ADSP_BUILT_INTERMEDIATES))
ifneq (yes,$(strip $(BUILD_WITH_XTENSA)))
TINYSYS_HEADER_FOLDERS += $(wildcard $(LOCAL_PATH)/../adsp_lib/$(MTK_PLATFORM_DIR))
else
TINYSYS_HEADER_FOLDERS += $(TINYSYS_ADSP_BUILT_INTERMEDIATES)/freertos/source/HIFI3_*
endif
endif

.PHONY: check-tinysys-config check-mtk-config
check-mtk-config: check-tinysys-config
check-tinysys-config: $(TINYSYS_SCP_CONFIG_HEADER) $(TINYSYS_ADSP_CONFIG_HEADER)
ifneq (yes,$(strip $(DISABLE_MTK_CONFIG_CHECK)))
	python device/mediatek/build/build/tools/check_kernel_config.py --prjconfig $(MTK_TARGET_PROJECT_FOLDER)/ProjectConfig.mk --project $(MTK_TARGET_PROJECT) --header `find $(TINYSYS_HEADER_FOLDERS) -type f -name tinysys_config.h | tr "\n" "," | sed -e "s/,$$//"`
else
	-python device/mediatek/build/build/tools/check_kernel_config.py --prjconfig $(MTK_TARGET_PROJECT_FOLDER)/ProjectConfig.mk --project $(MTK_TARGET_PROJECT) --header `find $(TINYSYS_HEADER_FOLDERS) -type f -name tinysys_config.h | tr "\n" "," | sed -e "s/,$$//"`
endif
endif
endif
