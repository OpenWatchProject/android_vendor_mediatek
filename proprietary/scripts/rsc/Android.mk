LOCAL_PATH := $(call my-dir)

my_rsc_script_path := $(LOCAL_PATH)

# $(1): base module name
# $(2): new module name suffix
# $(3): arm64 install path
# $(4): arm install path

define mtk-rsc-add-shared-library

ifneq ($$(TARGET_2ND_ARCH),)
include $$(CLEAR_VARS)
LOCAL_MODULE := $(1)$(2)
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PREBUILT_MODULE_FILE := $$(call intermediates-dir-for,SHARED_LIBRARIES,$(1),,,,)/$(1).so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_INSTALLED_MODULE_STEM := $(1).so
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_PATH := $(3)
include $(if $(3),$$(BUILD_PREBUILT))
endif

include $$(CLEAR_VARS)
LOCAL_MODULE := $(1)$(2)
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
ifneq ($$(TARGET_2ND_ARCH),)
LOCAL_PREBUILT_MODULE_FILE := $$(call intermediates-dir-for,SHARED_LIBRARIES,$(1),,,$$(TARGET_2ND_ARCH_VAR_PREFIX),)/$(1).so
else
LOCAL_PREBUILT_MODULE_FILE := $$(call intermediates-dir-for,SHARED_LIBRARIES,$(1),,,,)/$(1).so
endif
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_INSTALLED_MODULE_STEM := $(1).so
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_PATH := $(4)
include $(if $(4),$$(BUILD_PREBUILT))

endef


define mtk-rsc-add-executable

include $$(CLEAR_VARS)
LOCAL_MODULE := $(1)$(2)
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_PREBUILT_MODULE_FILE := $$(call intermediates-dir-for,EXECUTABLES,$(1),,,,)/$(1)
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 64
LOCAL_MODULE_PATH := $(3)
include $(if $(3),$$(BUILD_PREBUILT))

include $$(CLEAR_VARS)
LOCAL_MODULE := $(1)$(2)
LOCAL_MODULE_CLASS := EXECUTABLES
ifneq ($$(TARGET_2ND_ARCH),)
LOCAL_PREBUILT_MODULE_FILE := $$(call intermediates-dir-for,EXECUTABLES,$(1),,,$$(TARGET_2ND_ARCH_VAR_PREFIX),)/$(1)
else
LOCAL_PREBUILT_MODULE_FILE := $$(call intermediates-dir-for,EXECUTABLES,$(1),,,,)/$(1)
endif
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_PATH := $(4)
include $(if $(4),$$(BUILD_PREBUILT))

endef


define mtk-rsc-add-java-library

include $$(CLEAR_VARS)
LOCAL_MODULE := $(1)$(2)
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_PREBUILT_MODULE_FILE := $$(call intermediates-dir-for,JAVA_LIBRARIES,$(1),,,,)/javalib.jar
LOCAL_MULTILIB := first
LOCAL_INSTALLED_MODULE_STEM := $(1).jar
LOCAL_MODULE_SUFFIX := .jar
LOCAL_MODULE_PATH := $(3)
include $$(BUILD_PREBUILT)

endef


define mtk-rsc-add-package

include $$(CLEAR_VARS)
LOCAL_MODULE := $(1)$(2)
LOCAL_MODULE_CLASS := APPS
LOCAL_PREBUILT_MODULE_FILE := $$(call intermediates-dir-for,APPS,$(1),,,,)/package.apk
LOCAL_MULTILIB := first
LOCAL_INSTALLED_MODULE_STEM := $(1).apk
LOCAL_MODULE_SUFFIX := .apk
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_MODULE_PATH := $(3)/$(1)
LOCAL_IS_RUNTIME_RESOURCE_OVERLAY := true
LOCAL_DEX_PREOPT := $(4)
include $$(BUILD_PREBUILT)

$$(call intermediates-dir-for,APPS,$(1),,,,)/package.apk: dexpreopt-remove-classes.dex =

endef


mtk_rsc_override_modules :=
mtk_rsc_required_modules :=
mtk_rsc_check :=
define mtk_rsc_check_fail
$(eval mtk_rsc_check += error)\
$(warning $(1))
endef

$(foreach mk, $(CURRENT_MSSI_RSC_NAMES),\
  $(eval mssi_mtk_rsc_name := $(mk))\
  $(eval include device/mediatek/build/tasks/tools/build_mssi_mtk_rsc.mk))

$(foreach mk, $(CURRENT_RSC_NAMES),\
  $(eval mtk_rsc_name := $(mk))\
  $(eval include device/mediatek/build/tasks/tools/build_mtk_rsc.mk))


ifneq ($(strip $(mtk_rsc_check)),)
$(error RSC check fail)
endif


include $(CLEAR_VARS)
LOCAL_MODULE := rsc.xml
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_REQUIRED_MODULES := $(mtk_rsc_required_modules)
intermediates := $(call intermediates-dir-for,$(LOCAL_MODULE_CLASS),$(LOCAL_MODULE))

define generate-dummy-rsc-xml
	@mkdir -p $(dir $@)
	@echo "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" >$@
	@echo "<runtime_switchable_config version=\"1\">" >>$@
	@echo "</runtime_switchable_config>" >>$@
endef

$(intermediates)/sys.rsc.xml: $(MSSI_MTK_RSC_MAKEFILES)
	@mkdir -p $(dir $@)
	@echo "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" >$@
	@echo "<runtime_switchable_config version=\"1\">" >>$@
	@echo "<magic>0x5253434D</magic>" >>$@
ifdef SYS_TARGET_PROJECT
	@echo "  <proj_info>" >>$@
	@$(foreach m,$(CURRENT_MSSI_RSC_NAMES),echo "    <proj_item index=\"$(MSSI_MTK_RSC_XML_INDEX.$(m))\">" >>$@; echo "      <name>$(MSSI_MTK_RSC_XML_NAME.$(m))</name>" >>$@; echo "    </proj_item>" >>$@;)
	@echo "  </proj_info>" >>$@
endif
	@echo "</runtime_switchable_config>" >>$@

ifdef MTK_TARGET_PROJECT
$(intermediates)/vnd.rsc.xml: PRIVATE_PTGEN := $(PRODUCT_OUT)/$(MTK_PTGEN_CHIP)_Android_scatter.txt
$(intermediates)/vnd.rsc.xml: $(PRODUCT_OUT)/$(MTK_PTGEN_CHIP)_Android_scatter.txt
endif
$(intermediates)/vnd.rsc.xml: $(MTK_RSC_MAKEFILES)
	@mkdir -p $(dir $@)
	@echo "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" >$@
	@echo "<runtime_switchable_config version=\"1\">" >>$@
ifdef MTK_TARGET_PROJECT
ifneq ($(CURRENT_RSC_NAMES),)
	@echo "<magic>0x5253434D</magic>" >>$@
	@echo "  <part_info>" >>$@
	@echo -n "    <name>" >>$@
	@echo -n $$(cat $(PRIVATE_PTGEN) | grep "partition_name: \(para\|misc\)" | sed -e 's/  partition_name: //' | sort -r -u | head -n 1) >>$@
	@echo "</name>" >>$@
	@echo "    <offset>0x50000</offset>" >>$@
	@echo "  </part_info>" >>$@
	@echo "  <proj_info>" >>$@
	@$(foreach m,$(CURRENT_RSC_NAMES),echo "    <proj_item index=\"$(MTK_RSC_XML_INDEX.$(m))\">" >>$@; echo "      <name>$(MTK_RSC_XML_NAME.$(m))</name>" >>$@; $(foreach n,$(MTK_RSC_XML_OPTR.$(m)),echo "      <operator>$(n)</operator>" >>$@;) echo "    </proj_item>" >>$@;)
	@echo "  </proj_info>" >>$@
endif
endif
	@echo "</runtime_switchable_config>" >>$@

ifdef MTK_TARGET_PROJECT
$(intermediates)/krn.rsc.xml: $(TARGET_KERNEL_CONFIG) $(my_rsc_script_path)/rsc_xml_gen.py
	@mkdir -p $(dir $@)
	$(hide) python $(my_rsc_script_path)/rsc_xml_gen.py -kernel "$$(cat $(TARGET_KERNEL_CONFIG) | grep 'CONFIG_BUILD_ARM\w*_DTB_OVERLAY_IMAGE_NAMES=' | awk -F '=' '{print $$2}' | sed -e 's/mediatek\///g')" $@
else
$(intermediates)/krn.rsc.xml:
	$(call generate-dummy-rsc-xml)
endif

$(intermediates)/usr.rsc.xml:
	$(call generate-dummy-rsc-xml)

$(intermediates)/dummy.rsc.xml:
	@mkdir -p $(dir $@)
	$(hide) touch $@

$(intermediates)/gen.xml: PRIVATE_FLAGS := $(intermediates)/sys.rsc.xml $(intermediates)/vnd.rsc.xml $(intermediates)/krn.rsc.xml $(intermediates)/usr.rsc.xml
$(intermediates)/gen.xml: $(intermediates)/sys.rsc.xml $(intermediates)/vnd.rsc.xml $(intermediates)/krn.rsc.xml $(intermediates)/usr.rsc.xml $(my_rsc_script_path)/rsc_xml_gen.py
	@mkdir -p $(dir $@)
	$(hide) python $(my_rsc_script_path)/rsc_xml_gen.py -final $(PRIVATE_FLAGS) $@


LOCAL_PREBUILT_MODULE_FILE := $(intermediates)/dummy.rsc.xml
LOCAL_UNINSTALLABLE_MODULE := true
ifdef SYS_TARGET_PROJECT
  ifdef MTK_TARGET_PROJECT
    # virtual
    LOCAL_PREBUILT_MODULE_FILE := $(intermediates)/gen.xml
    ifeq ($(filter 0 1,$(words $(CURRENT_MSSI_RSC_NAMES))),)
    ifeq ($(filter 0 1,$(words $(CURRENT_RSC_NAMES))),)
      LOCAL_UNINSTALLABLE_MODULE :=
      LOCAL_MODULE_PATH := $(TARGET_OUT)/$(PRODUCT_MSSI_MTK_RSC_ROOT_PATH)
      LOCAL_POST_INSTALL_CMD := cp -f $(LOCAL_PREBUILT_MODULE_FILE) $(PRODUCT_OUT)/$(LOCAL_MODULE)
    endif
    endif
  else
    # system
    ifeq ($(filter 0 1,$(words $(CURRENT_MSSI_RSC_NAMES))),)
      LOCAL_UNINSTALLABLE_MODULE :=
      LOCAL_MODULE_PATH := $(TARGET_OUT)/$(PRODUCT_MSSI_MTK_RSC_ROOT_PATH)
    endif
  endif
else
  ifdef MTK_TARGET_PROJECT
    ifneq ($(support_split_build),true)
      # legacy
      LOCAL_PREBUILT_MODULE_FILE := $(intermediates)/vnd.rsc.xml
      ifeq ($(filter 0 1,$(words $(CURRENT_RSC_NAMES))),)
        LOCAL_UNINSTALLABLE_MODULE :=
        LOCAL_MODULE_PATH := $(TARGET_OUT)/$(PRODUCT_MTK_RSC_ROOT_PATH)
        LOCAL_POST_INSTALL_CMD := cp -f $(LOCAL_PREBUILT_MODULE_FILE) $(PRODUCT_OUT)/$(LOCAL_MODULE)
      endif
    else
      # vendor
    endif
  endif
endif
include $(BUILD_PREBUILT)

PACKAGES.rsc.xml.OVERRIDES := $(mtk_rsc_override_modules)


include $(CLEAR_VARS)
LOCAL_MODULE := check-rsc-config
LOCAL_MODULE_CLASS := FAKE
LOCAL_ADDITIONAL_DEPENDENCIES += $(call local-intermediates-dir)/rsc.txt
# rule for rsc.txt is in device/mediatek/build/tasks/mtk_rsc.mk
include $(BUILD_PHONY_PACKAGE)

