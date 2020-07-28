### cgen module

intermediates := $(call intermediates-dir-for,ETC,apdb)
MTK_CGEN_OUT_DIR := $(intermediates)
MTK_CGEN_BIN_DIR := vendor/mediatek/proprietary/cgen
MTK_CGEN_CUSTOM_DIR := vendor/mediatek/proprietary/custom/$(strip $(MTK_PROJECT))
MTK_CGEN_CUSTOM_PROJECT_DIR := vendor/mediatek/proprietary/custom/$(strip $(MTK_PROJECT))/cgen
MTK_CGEN_CUSTOM_PLATFORM_DIR := vendor/mediatek/proprietary/custom/$(subst M,m,$(subst T,t,$(strip $(MTK_PLATFORM))))/cgen
MTK_CGEN_CUSTOM_COMMON_DIR := vendor/mediatek/proprietary/custom/common/cgen

MTK_CGEN_COMPILE_OPTION := $(strip $(MTK_INTERNAL_CDEFS))
MTK_CGEN_COMPILE_OPTION += -I$(MTK_CGEN_OUT_DIR)/inc -I$(MTK_CGEN_BIN_DIR)/apeditor

MTK_CGEN_APDB_SourceFile := $(MTK_CGEN_OUT_DIR)/apeditor/app_parse_db.c
MTK_CGEN_AP_Temp_CL	:= $(MTK_CGEN_OUT_DIR)/apeditor/app_temp_db

MTK_CGEN_TARGET_CFG	:= $(MTK_CGEN_BIN_DIR)/cgencfg/tgt_cnf
MTK_CGEN_HOST_CFG	:= $(MTK_CGEN_BIN_DIR)/cgencfg/pc_cnf
MTK_CGEN_EXECUTABLE	:= $(MTK_CGEN_BIN_DIR)/Cgen

MTK_CGEN_cfg_module_file_dir	:= $(MTK_CGEN_CUSTOM_PLATFORM_DIR)/cfgfileinc $(MTK_CGEN_CUSTOM_COMMON_DIR)/cfgfileinc
MTK_CGEN_cfg_module_default_dir	:= $(MTK_CGEN_CUSTOM_PLATFORM_DIR)/cfgdefault $(MTK_CGEN_CUSTOM_COMMON_DIR)/cfgdefault
MTK_CGEN_custom_cfg_module_file_dir     := $(MTK_CGEN_CUSTOM_PROJECT_DIR)/cfgfileinc
MTK_CGEN_custom_cfg_default_file_dir    := $(MTK_CGEN_CUSTOM_PROJECT_DIR)/cfgdefault

MTK_CGEN_cfg_module_file	:= $(MTK_CGEN_OUT_DIR)/inc/cfg_module_file.h
MTK_CGEN_cfg_module_default	:= $(MTK_CGEN_OUT_DIR)/inc/cfg_module_default.h
MTK_CGEN_custom_cfg_module_file		:= $(MTK_CGEN_OUT_DIR)/inc/custom_cfg_module_file.h
MTK_CGEN_custom_cfg_default_file	:= $(MTK_CGEN_OUT_DIR)/inc/custom_cfg_module_default.h

MTK_CGEN_AP_Editor_BRANCH := $(strip $(patsubst ro.mediatek.version.branch=%,%,$(firstword $(filter ro.mediatek.version.branch=%,$(PRODUCT_PROPERTY_OVERRIDES)))))
MTK_CGEN_AP_Editor_DB		:= $(MTK_CGEN_OUT_DIR)/APDB_$(MTK_PLATFORM)_$(MTK_CHIP_VER)_$(MTK_CGEN_AP_Editor_BRANCH)_$(MTK_INTERNAL_WEEK_NO)
MTK_CGEN_AP_Editor2_Temp_DB	:= $(MTK_CGEN_OUT_DIR)/.temp_APDB_$(MTK_PLATFORM)_$(MTK_CHIP_VER)_$(MTK_CGEN_AP_Editor_BRANCH)_$(MTK_INTERNAL_WEEK_NO)
MTK_CGEN_AP_Editor_DB_Enum_File	:= $(MTK_CGEN_OUT_DIR)/APDB_$(MTK_PLATFORM)_$(MTK_CHIP_VER)_$(MTK_CGEN_AP_Editor_BRANCH)_$(MTK_INTERNAL_WEEK_NO)_ENUM

ifneq ($(strip $(TARGET_BOARD_PLATFORM)),)
  MTK_CGEN_hardWareVersion := $(subst m,M,$(subst t,T,$(strip $(TARGET_BOARD_PLATFORM))))_$(MTK_CHIP_VER)
else
  MTK_CGEN_hardWareVersion := $(MTK_PLATFORM)_$(MTK_CHIP_VER)
endif
MTK_CGEN_SWVersion := $(MTK_INTERNAL_BUILD_VERNO)


# $(1): $dir_path, the searched directory
# $(2): $fileName, the generated header file name
# $(3): $out_dir
define mtk-cgen-AutoGenHeaderFile
$(2): $$(foreach d,$(1),$$(sort $$(wildcard $$(d)/*.h))) $(4)
	@echo Cgen: $$@
	$(hide) rm -f $$@
	$(hide) mkdir -p $$(dir $$@)
	$(hide) for x in $$(foreach d,$(1),$$(sort $$(wildcard $$(d)/*.h))); do echo "#include \"$(3)$$$$x\"" >>$$@; done;
	$(hide) for x in $(4); do echo "#include \"$$$$x\"" >>$$@; done;
endef

define mtk-cgen-PreprocessFile
$(2): .KATI_DEPFILE := $$(basename $(2)).d
$(2): $(1)
	@echo Cgen: $$@
	@mkdir -p $$(dir $(2))
	$(hide) $(CLANG) $(3) -E $(1) -o $(2) -MMD -MF $$(basename $(2)).d -MQ '$(2)'
endef


$(eval $(call mtk-cgen-AutoGenHeaderFile,$(MTK_CGEN_cfg_module_file_dir),$(MTK_CGEN_cfg_module_file)))
$(eval $(call mtk-cgen-AutoGenHeaderFile,$(MTK_CGEN_cfg_module_default_dir),$(MTK_CGEN_cfg_module_default)))
$(eval $(call mtk-cgen-AutoGenHeaderFile,$(MTK_CGEN_custom_cfg_module_file_dir),$(MTK_CGEN_custom_cfg_module_file)))
$(eval $(call mtk-cgen-AutoGenHeaderFile,$(MTK_CGEN_custom_cfg_default_file_dir),$(MTK_CGEN_custom_cfg_default_file)))

$(eval $(call mtk-cgen-PreprocessFile,$(MTK_CGEN_APDB_SourceFile),$(MTK_CGEN_AP_Temp_CL),$(MTK_CGEN_COMPILE_OPTION) -I. -I$(MTK_CGEN_CUSTOM_PROJECT_DIR)/inc -I$(MTK_CGEN_CUSTOM_PLATFORM_DIR)/inc -I$(MTK_CGEN_CUSTOM_COMMON_DIR)/inc))

$(MTK_CGEN_APDB_SourceFile):
	@echo Cgen: $@
	$(hide) rm -f $@
	$(hide) mkdir -p $(dir $@)
	$(hide) echo \#include \"tst_assert_header_file.h\"	>$@
	$(hide) echo \#include \"ap_editor_data_item.h\"	>>$@
	$(hide) echo \#include \"Custom_NvRam_data_item.h\"	>>$@

$(MTK_CGEN_AP_Temp_CL): $(MTK_CGEN_cfg_module_file)
$(MTK_CGEN_AP_Temp_CL): $(MTK_CGEN_cfg_module_default)
$(MTK_CGEN_AP_Temp_CL): $(MTK_CGEN_custom_cfg_module_file)
$(MTK_CGEN_AP_Temp_CL): $(MTK_CGEN_custom_cfg_default_file)

$(MTK_CGEN_AP_Editor2_Temp_DB): $(MTK_CGEN_EXECUTABLE) $(MTK_CGEN_TARGET_CFG) $(MTK_CGEN_HOST_CFG) $(MTK_CGEN_AP_Temp_CL)
	@echo Cgen: $@
	$(hide) $(MTK_CGEN_EXECUTABLE) -c $(MTK_CGEN_AP_Temp_CL) $(MTK_CGEN_TARGET_CFG) $(MTK_CGEN_HOST_CFG) $(MTK_CGEN_AP_Editor2_Temp_DB) $(MTK_CGEN_AP_Editor_DB_Enum_File) $(MTK_CGEN_hardWareVersion) $(MTK_CGEN_SWVersion)

$(MTK_CGEN_AP_Editor_DB): $(MTK_CGEN_EXECUTABLE) $(MTK_CGEN_AP_Editor2_Temp_DB) $(MTK_CGEN_AP_Temp_CL)
	@echo Cgen: $@
	$(hide) $(MTK_CGEN_EXECUTABLE) -cm $(MTK_CGEN_AP_Editor_DB) $(MTK_CGEN_AP_Editor2_Temp_DB) $(MTK_CGEN_AP_Temp_CL) $(MTK_CGEN_AP_Editor_DB_Enum_File) $(MTK_CGEN_hardWareVersion) $(MTK_CGEN_SWVersion)


LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

CGEN_MODULE := apdb
LOCAL_MODULE := $(CGEN_MODULE)
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_STEM := APDB_$(MTK_PLATFORM)_$(MTK_CHIP_VER)_$(MTK_CGEN_AP_Editor_BRANCH)_$(MTK_INTERNAL_WEEK_NO)
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := apdb
LOCAL_POST_INSTALL_CMD := cp -f $(MTK_CGEN_AP_Editor_DB_Enum_File) $(TARGET_OUT_VENDOR)/etc/apdb

include $(BUILD_SYSTEM)/base_rules.mk
