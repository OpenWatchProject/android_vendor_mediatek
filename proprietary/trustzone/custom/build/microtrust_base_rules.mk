ifeq ($(LOCAL_MAKEFILE),)
  $(error LOCAL_MAKEFILE is not defined)
endif
ifeq ($(TARGET),)
  $(error $(LOCAL_MAKEFILE): TARGET is not defined)
endif

LOCAL_MODULE_CLASS := $(notdir $(lastword $(filter-out $(MICROTRUST_BASE_RULES),$(MAKEFILE_LIST))))
ifneq ($(filter $(LOCAL_MODULE_CLASS),prog.mk prog_spec.mk),)
  ifneq ($(filter %.drv,$(TARGET)),)
    module_id := $(patsubst %.drv,%,$(filter %.drv,$(TARGET)))
  else ifneq ($(filter %.ta,$(TARGET)),)
    module_id := $(patsubst %.ta,%,$(filter %.ta,$(TARGET)))
  else
    module_id := $(strip $(TARGET))
  endif
else ifeq ($(LOCAL_MODULE_CLASS),lib.mk)
  ifneq ($(filter %.a,$(TARGET)),)
    module_id := $(patsubst %.a,%,$(filter %.a,$(TARGET)))
  else ifneq ($(filter %.so,$(TARGET)),)
    module_id := $(patsubst %.so,%,$(filter %.so,$(TARGET)))
  else
    module_id := $(strip $(TARGET))
  endif
else
  $(error $(LOCAL_MAKEFILE): prog.mk or prog_spec.mk or lib.mk are not included)
endif

ifneq ($(filter $(module_id),$(MICROTRUST_ALL_MODULES)),)
  ifneq ($(LOCAL_MAKEFILE),$(MICROTRUST_ALL_MODULES.$(module_id).MAKEFILE))
    $(error $(LOCAL_MAKEFILE): $(module_id) already defined by $(MICROTRUST_ALL_MODULES.$(module_id).MAKEFILE))
  endif
endif

MICROTRUST_ALL_MODULES := $(MICROTRUST_ALL_MODULES) $(module_id)
MICROTRUST_ALL_MODULES.$(module_id).MAKEFILE := $(LOCAL_MAKEFILE)
MICROTRUST_ALL_MODULES.$(module_id).PATH := $(patsubst %/,%,$(dir $(LOCAL_MAKEFILE)))
MICROTRUST_ALL_MODULES.$(module_id).OUTPUT_ROOT := $(MICROTRUST_OUTPUT_PATH)/$(module_id)
MICROTRUST_HAL_DEPENDENCIES :=
imported_includes :=

ifneq ($(filter $(LOCAL_MODULE_CLASS),prog.mk prog_spec.mk),)
    MICROTRUST_ALL_MODULES.$(module_id).BUILT := $(MICROTRUST_ALL_MODULES.$(module_id).OUTPUT_ROOT)/$(if $(strip $(TARGET_FILE_NAME)),$(strip $(TARGET_FILE_NAME)),$(strip $(TARGET)))
    MICROTRUST_ALL_MODULES.$(module_id).INSTALLED := $(MICROTRUST_INSTALL_PATH)/$(if $(strip $(TARGET_FILE_NAME)),$(strip $(TARGET_FILE_NAME)),$(strip $(TARGET)))
  ifneq ($(EXTERNAL_LIB),)
    MICROTRUST_ALL_MODULES.$(module_id).REQUIRED := $(strip $(MICROTRUST_ALL_MODULES.$(module_id).REQUIRED) $(patsubst lib%.a,lib%,$(filter lib%.a,$(notdir $(EXTERNAL_LIB)))))
  endif
  ifneq ($(MY_LDFLAGS),)
    MICROTRUST_ALL_MODULES.$(module_id).REQUIRED := $(strip $(MICROTRUST_ALL_MODULES.$(module_id).REQUIRED) $(patsubst -l%,lib%,$(filter -l%,$(MY_LDFLAGS))))
  endif
else ifeq ($(LOCAL_MODULE_CLASS),lib.mk)
  ifneq ($(filter %.a,$(TARGET)),)
    MICROTRUST_ALL_MODULES.$(module_id).LIB := $(MICROTRUST_ALL_MODULES.$(module_id).OUTPUT_ROOT)/$(filter %.a,$(TARGET))
    MICROTRUST_ALL_MODULES.$(module_id).BUILT := $(MICROTRUST_ALL_MODULES.$(module_id).LIB)
  endif
  ifneq ($(filter %.so,$(TARGET)),)
    MICROTRUST_ALL_MODULES.$(module_id).BUILT := $(MICROTRUST_ALL_MODULES.$(module_id).OUTPUT_ROOT)/$(filter %.so,$(TARGET))
    MICROTRUST_ALL_MODULES.$(module_id).INSTALLED := $(MICROTRUST_INSTALL_PATH)/$(filter %.so,$(TARGET))
  endif
endif
ifneq ($(strip $(HAL_LIBS)),)
  MICROTRUST_ALL_MODULES.$(module_id).HAL_LIBS := $(notdir $(filter $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/%,$(HAL_LIBS)))
  my_static_libraries := $(basename $(MICROTRUST_ALL_MODULES.$(module_id).HAL_LIBS))
  ifdef TARGET_2ND_ARCH
    MICROTRUST_HAL_DEPENDENCIES := $(foreach l,$(my_static_libraries),$(call intermediates-dir-for,STATIC_LIBRARIES,$(l),,,$(TARGET_2ND_ARCH_VAR_PREFIX))/$(l).lib)
    ifneq ($(PLATFORM_VERSION),R)
      imported_includes := $(strip $(foreach l,$(my_static_libraries),$(call intermediates-dir-for,STATIC_LIBRARIES,$(l),,,$(TARGET_2ND_ARCH_VAR_PREFIX))))
    endif
  else
    MICROTRUST_HAL_DEPENDENCIES := $(foreach l,$(my_static_libraries),$(call intermediates-dir-for,STATIC_LIBRARIES,$(l))/$(l).lib)
    ifneq ($(PLATFORM_VERSION),R)
      imported_includes := $(strip $(foreach l,$(my_static_libraries),$(call intermediates-dir-for,STATIC_LIBRARIES,$(l))))
    endif
  endif
  MICROTRUST_HAL_DEPENDENCIES += $(addsuffix /export_includes,$(imported_includes))
endif

MICROTRUST_MAKE_DEPENDENCIES := $(shell?find $(LOCAL_PATH) -name .git -prune -o -type f | sort)

.KATI_RESTAT: $(MICROTRUST_ALL_MODULES.$(module_id).BUILT)
$(MICROTRUST_ALL_MODULES.$(module_id).BUILT): PRIVATE_MAKEFILE := $(abspath $(MICROTRUST_ALL_MODULES.$(module_id).MAKEFILE))
$(MICROTRUST_ALL_MODULES.$(module_id).BUILT): PRIVATE_PATH := $(abspath $(MICROTRUST_ALL_MODULES.$(module_id).PATH))
$(MICROTRUST_ALL_MODULES.$(module_id).BUILT): PRIVATE_MAKE_OPTION := $(MICROTRUST_GLOBAL_MAKE_OPTION)
$(MICROTRUST_ALL_MODULES.$(module_id).BUILT): PRIVATE_MAKE_OPTION += LOCAL_OBJS_DIR=$(abspath $(MICROTRUST_ALL_MODULES.$(module_id).OUTPUT_ROOT))
$(MICROTRUST_ALL_MODULES.$(module_id).BUILT): PRIVATE_MAKE_OPTION += $(MICROTRUST_GLOBAL_PLATFORM_OPTION)
$(MICROTRUST_ALL_MODULES.$(module_id).BUILT): $(MICROTRUST_MAKE_DEPENDENCIES)
$(MICROTRUST_ALL_MODULES.$(module_id).BUILT): $(MICROTRUST_HAL_DEPENDENCIES)
	@echo Microtrust build: $@
	$(hide) mkdir -p $(dir $@)
	$(PREBUILT_MAKE_PREFIX)$(MAKE) -r -R -C $(PRIVATE_PATH) -f $(PRIVATE_MAKEFILE) $(PRIVATE_MAKE_OPTION) all

ifeq ($(MICROTRUST_ALL_MODULES.$(module_id).INSTALLED),)
MICROTRUST_modules_to_check := $(MICROTRUST_modules_to_check) $(MICROTRUST_ALL_MODULES.$(module_id).BUILT)
else ifneq ($(strip $(MICROTRUST_TEE_SUPPORT)),yes)
MICROTRUST_modules_to_check := $(MICROTRUST_modules_to_check) $(MICROTRUST_ALL_MODULES.$(module_id).BUILT)
else
MICROTRUST_modules_to_install := $(MICROTRUST_modules_to_install) $(MICROTRUST_ALL_MODULES.$(module_id).INSTALLED)
$(MICROTRUST_ALL_MODULES.$(module_id).INSTALLED): $(MICROTRUST_ALL_MODULES.$(module_id).BUILT)
	@echo Signing: $@
	$(hide) mkdir -p $(dir $@)
#	$(hide) python -O $(UT_SDK_HOME)/Signature_tool/sign_tool/sign.pyo $(dir $<)$(notdir $@) 1 $@ 654321
	$(hide) sh $(UT_SDK_HOME)/Signature_tool/sign_tool/sign.sh $(UT_SDK_HOME) $(dir $<)$(notdir $@) $@
#	@echo Copying: $@
#	$(hide) cp -f $(dir $<)$(notdir $@) $@
endif

ifeq (dump,dump)
MICROTRUST_dumpvar_log := $(MICROTRUST_OUTPUT_PATH)/dump/$(module_id).log
$(MICROTRUST_dumpvar_log): DUMPVAR_VALUE := MAKEFILE=$(MICROTRUST_ALL_MODULES.$(module_id).MAKEFILE)
$(MICROTRUST_dumpvar_log): DUMPVAR_VALUE += PATH=$(MICROTRUST_ALL_MODULES.$(module_id).PATH)
$(MICROTRUST_dumpvar_log): DUMPVAR_VALUE += BUILT=$(MICROTRUST_ALL_MODULES.$(module_id).BUILT)
$(MICROTRUST_dumpvar_log): DUMPVAR_VALUE += INSTALLED=$(MICROTRUST_ALL_MODULES.$(module_id).INSTALLED)
$(MICROTRUST_dumpvar_log): PRIVATE_TARGET := $(strip $(TARGET))
$(MICROTRUST_dumpvar_log): PRIVATE_REQUIRED := $(MICROTRUST_ALL_MODULES.$(module_id).REQUIRED)
$(MICROTRUST_dumpvar_log): PRIVATE_HAL_LIBS := $(MICROTRUST_ALL_MODULES.$(module_id).HAL_LIBS)
MICROTRUST_modules_to_check := $(MICROTRUST_modules_to_check) $(MICROTRUST_dumpvar_log)
$(MICROTRUST_dumpvar_log): $(MICROTRUST_ALL_MODULES.$(module_id).MAKEFILE) $(MICROTRUST_BASE_RULES)
	@echo Microtrust dump: $@
	$(hide) mkdir -p $(dir $@)
	$(hide) rm -f $@
	$(hide) $(foreach v,$(DUMPVAR_VALUE),echo $(v) >>$@;)
	$(hide) echo TARGET=$(PRIVATE_TARGET) >>$@
	$(hide) echo REQUIRED=$(PRIVATE_REQUIRED) >>$@
	$(hide) echo HAL_LIBS=$(PRIVATE_HAL_LIBS) >>$@

endif
