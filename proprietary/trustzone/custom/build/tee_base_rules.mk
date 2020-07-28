ifeq ($(LOCAL_MAKEFILE),)
  $(error LOCAL_MAKEFILE is not defined)
endif
ifeq ($(OUTPUT_NAME),)
  $(error $(LOCAL_MAKEFILE): OUTPUT_NAME is not defined)
endif
ifeq ($(TEE_MODE),)
  $(error $(LOCAL_MAKEFILE): TEE_MODE is not defined)
endif
ifneq ($(strip $(DRIVER_UUID)),)
  ifeq ($(strip $(BUILD_DRIVER_LIBRARY_ONLY)),YES)
    module_id := $(OUTPUT_NAME).lib
  else ifneq ($(strip $(SRC_LIB_C)),)
    module_id := $(OUTPUT_NAME).lib
  else ifneq ($(strip $(SRC_CPP))$(strip $(SRC_C))$(strip $(SRC_ASM)),)
    module_id := $(OUTPUT_NAME).drbin
  else ifneq ($(wildcard $(foreach n,$(TEE_MODE) $(call LowerCase,$(TEE_MODE)) release debug,$(dir $(LOCAL_MAKEFILE))$(n)/$(OUTPUT_NAME).lib)),)
    module_id := $(OUTPUT_NAME).lib
  else
    module_id := $(OUTPUT_NAME).drbin
  endif
else ifneq ($(strip $(TRUSTLET_UUID)),)
  ifeq ($(BUILD_TRUSTLET_LIBRARY_ONLY),yes)
    module_id := $(OUTPUT_NAME).lib
  else ifeq ($(GP_ENTRYPOINTS),Y)
    module_id := $(OUTPUT_NAME).tabin
  else
    module_id := $(OUTPUT_NAME).tlbin
  endif
else
  $(error $(LOCAL_MAKEFILE): DRIVER_UUID and TRUSTLET_UUID are not defined)
endif

ifneq ($(filter $(module_id),$(TEE_ALL_MODULES)),)
  ifneq ($(LOCAL_MAKEFILE),$(TEE_ALL_MODULES.$(module_id).MAKEFILE))
    $(error $(LOCAL_MAKEFILE): $(module_id) already defined by $(TEE_ALL_MODULES.$(module_id).MAKEFILE))
  endif
endif

TEE_ALL_MODULES := $(TEE_ALL_MODULES) $(module_id)
TEE_ALL_MODULES.$(module_id).MAKEFILE := $(LOCAL_MAKEFILE)
TEE_ALL_MODULES.$(module_id).PATH := $(patsubst %/makefile.mk,%,$(patsubst %/Locals/Code/makefile.mk,%,$(LOCAL_MAKEFILE)))
TEE_HAL_DEPENDENCIES :=
imported_includes :=

ifneq ($(strip $(DRIVER_UUID)),)
    TEE_ALL_MODULES.$(module_id).CLASS := DRIVER
    TEE_ALL_MODULES.$(module_id).OUTPUT_ROOT := $(TEE_DRIVER_OUTPUT_PATH)/$(OUTPUT_NAME)
  ifeq ($(strip $(BUILD_DRIVER_LIBRARY_ONLY)),YES)
    TEE_ALL_MODULES.$(module_id).$(TEE_MODE).BUILT := $(TEE_DRIVER_OUTPUT_PATH)/$(OUTPUT_NAME)/$(TEE_MODE)/$(OUTPUT_NAME).lib
  else
    TEE_ALL_MODULES.$(module_id).$(TEE_MODE).AXF := $(TEE_DRIVER_OUTPUT_PATH)/$(OUTPUT_NAME)/$(TEE_MODE)/$(OUTPUT_NAME).axf
    TEE_ALL_MODULES.$(module_id).$(TEE_MODE).BUILT := $(TEE_DRIVER_OUTPUT_PATH)/$(OUTPUT_NAME)/$(TEE_MODE)/$(strip $(DRIVER_UUID)).drbin
    TEE_ALL_MODULES.$(module_id).INSTALLED := $(TEE_APP_INSTALL_PATH)/$(strip $(DRIVER_UUID)).drbin $(TEE_APP_INSTALL_PATH)/$(strip $(DRIVER_UUID)).tlbin
  endif
  ifeq ($(BUILD_DRIVER_LIBRARY_ONLY),YES)
    TEE_ALL_MODULES.$(module_id).$(TEE_MODE).LIB := $(TEE_DRIVER_OUTPUT_PATH)/$(OUTPUT_NAME)/$(TEE_MODE)/$(OUTPUT_NAME).lib
  else ifneq ($(strip $(SRC_LIB_C)),)
    TEE_ALL_MODULES.$(module_id).$(TEE_MODE).LIB := $(TEE_DRIVER_OUTPUT_PATH)/$(OUTPUT_NAME)/$(TEE_MODE)/$(OUTPUT_NAME).lib
  endif
  ifneq ($(strip $(EXTRA_LIBS)),)
    TEE_ALL_MODULES.$(module_id).$(TEE_MODE).REQUIRED := $(notdir $(EXTRA_LIBS))
  endif
else ifneq ($(strip $(TRUSTLET_UUID)),)
    TEE_ALL_MODULES.$(module_id).CLASS := TRUSTLET
    TEE_ALL_MODULES.$(module_id).OUTPUT_ROOT := $(TEE_TRUSTLET_OUTPUT_PATH)/$(OUTPUT_NAME)
  ifeq ($(BUILD_TRUSTLET_LIBRARY_ONLY),yes)
    TEE_ALL_MODULES.$(module_id).$(TEE_MODE).LIB := $(TEE_TRUSTLET_OUTPUT_PATH)/$(OUTPUT_NAME)/$(TEE_MODE)/$(OUTPUT_NAME).lib
    TEE_ALL_MODULES.$(module_id).$(TEE_MODE).BUILT := $(TEE_TRUSTLET_OUTPUT_PATH)/$(OUTPUT_NAME)/$(TEE_MODE)/$(OUTPUT_NAME).lib
  else ifeq ($(GP_ENTRYPOINTS),Y)
    TEE_ALL_MODULES.$(module_id).$(TEE_MODE).AXF := $(TEE_TRUSTLET_OUTPUT_PATH)/$(OUTPUT_NAME)/$(TEE_MODE)/$(OUTPUT_NAME).axf
    TEE_ALL_MODULES.$(module_id).$(TEE_MODE).BUILT := $(TEE_TRUSTLET_OUTPUT_PATH)/$(OUTPUT_NAME)/$(TEE_MODE)/$(strip $(TRUSTLET_UUID)).tabin
    TEE_ALL_MODULES.$(module_id).INSTALLED := $(TEE_APP_INSTALL_PATH)/$(strip $(TRUSTLET_UUID)).tabin
  else
    TEE_ALL_MODULES.$(module_id).$(TEE_MODE).AXF := $(TEE_TRUSTLET_OUTPUT_PATH)/$(OUTPUT_NAME)/$(TEE_MODE)/$(OUTPUT_NAME).axf
    TEE_ALL_MODULES.$(module_id).$(TEE_MODE).BUILT := $(TEE_TRUSTLET_OUTPUT_PATH)/$(OUTPUT_NAME)/$(TEE_MODE)/$(strip $(TRUSTLET_UUID)).tlbin
    TEE_ALL_MODULES.$(module_id).INSTALLED := $(TEE_APP_INSTALL_PATH)/$(strip $(TRUSTLET_UUID)).tlbin
  endif
  ifneq ($(strip $(CUSTOMER_DRIVER_LIBS)),)
    TEE_ALL_MODULES.$(module_id).$(TEE_MODE).REQUIRED := $(notdir $(CUSTOMER_DRIVER_LIBS))
  endif
else
  $(error $(LOCAL_MAKEFILE): DRIVER_UUID and TRUSTLET_UUID are not defined)
endif
ifneq ($(strip $(HAL_LIBS)),)
  TEE_ALL_MODULES.$(module_id).$(TEE_MODE).HAL_LIBS := $(notdir $(filter $(ANDROID_STATIC_LIBRARIES_OUT_DIR)/%,$(HAL_LIBS)))
  my_static_libraries := $(basename $(TEE_ALL_MODULES.$(module_id).$(TEE_MODE).HAL_LIBS))
  ifdef TARGET_2ND_ARCH
    TEE_HAL_DEPENDENCIES := $(foreach l,$(my_static_libraries),$(call intermediates-dir-for,STATIC_LIBRARIES,$(l),,,$(TARGET_2ND_ARCH_VAR_PREFIX))/$(l).lib)
    ifneq ($(PLATFORM_VERSION),R)
      imported_includes := $(strip $(foreach l,$(my_static_libraries),$(call intermediates-dir-for,STATIC_LIBRARIES,$(l),,,$(TARGET_2ND_ARCH_VAR_PREFIX))))
    endif
  else
    TEE_HAL_DEPENDENCIES := $(foreach l,$(my_static_libraries),$(call intermediates-dir-for,STATIC_LIBRARIES,$(l))/$(l).lib)
    ifneq ($(PLATFORM_VERSION),R)
      imported_includes := $(strip $(foreach l,$(my_static_libraries),$(call intermediates-dir-for,STATIC_LIBRARIES,$(l))))
    endif
  endif
  TEE_HAL_DEPENDENCIES += $(addsuffix /export_includes,$(imported_includes))
endif

TEE_MAKE_DEPENDENCIES := $(shell find $(LOCAL_PATH) -type f -and -not -name ".*")

.KATI_RESTAT: $(TEE_ALL_MODULES.$(module_id).$(TEE_MODE).BUILT)
$(TEE_ALL_MODULES.$(module_id).$(TEE_MODE).BUILT): PRIVATE_PATH := $(TRUSTZONE_ROOT_DIR)/$(TEE_ALL_MODULES.$(module_id).PATH)
$(TEE_ALL_MODULES.$(module_id).$(TEE_MODE).BUILT): PRIVATE_MAKEFILE := $(TRUSTZONE_ROOT_DIR)/$(TEE_ALL_MODULES.$(module_id).MAKEFILE)
$(TEE_ALL_MODULES.$(module_id).$(TEE_MODE).BUILT): PRIVATE_MAKE_OPTION := $(TEE_GLOBAL_MAKE_OPTION)
$(TEE_ALL_MODULES.$(module_id).$(TEE_MODE).BUILT): PRIVATE_MAKE_OPTION += TOOLCHAIN=$(TEE_TOOLCHAIN)
$(TEE_ALL_MODULES.$(module_id).$(TEE_MODE).BUILT): PRIVATE_MAKE_OPTION += MODE=$(TEE_MODE)
$(TEE_ALL_MODULES.$(module_id).$(TEE_MODE).BUILT): PRIVATE_MAKE_OPTION += TEE_MODE=$(TEE_MODE)
$(TEE_ALL_MODULES.$(module_id).$(TEE_MODE).BUILT): PRIVATE_MAKE_OPTION += OUTPUT_ROOT=$(if $(filter ~% /%,$(TRUSTZONE_OUTPUT_PATH)),,$(TRUSTZONE_ROOT_DIR)/)$(TEE_ALL_MODULES.$(module_id).OUTPUT_ROOT)
$(TEE_ALL_MODULES.$(module_id).$(TEE_MODE).BUILT): PRIVATE_MAKE_OPTION += TEE_DRIVER_OUTPUT_PATH=$(if $(filter ~% /%,$(TRUSTZONE_OUTPUT_PATH)),,$(TRUSTZONE_ROOT_DIR)/)$(TEE_DRIVER_OUTPUT_PATH)
$(TEE_ALL_MODULES.$(module_id).$(TEE_MODE).BUILT): PRIVATE_MAKE_OPTION += $(TEE_GLOBAL_PLATFORM_OPTION)

$(TEE_ALL_MODULES.$(module_id).$(TEE_MODE).BUILT): $(TEE_MAKE_DEPENDENCIES)
$(TEE_ALL_MODULES.$(module_id).$(TEE_MODE).BUILT): $(TEE_HAL_DEPENDENCIES)
	@echo TEE build: $@
	$(PREBUILT_MAKE_PREFIX)$(MAKE) -C $(PRIVATE_PATH) -f $(PRIVATE_MAKEFILE) $(PRIVATE_MAKE_OPTION) all

ifeq ($(TEE_ALL_MODULES.$(module_id).INSTALLED),)
TEE_modules_to_check := $(TEE_modules_to_check) $(TEE_ALL_MODULES.$(module_id).$(TEE_MODE).BUILT)
else ifneq ($(TEE_MODE),$(strip $(TEE_INSTALL_MODE)))
TEE_modules_to_check := $(TEE_modules_to_check) $(TEE_ALL_MODULES.$(module_id).$(TEE_MODE).BUILT)
else ifneq ($(strip $(TRUSTONIC_TEE_SUPPORT)),yes)
TEE_modules_to_check := $(TEE_modules_to_check) $(TEE_ALL_MODULES.$(module_id).$(TEE_MODE).BUILT)
else
TEE_modules_to_install := $(TEE_modules_to_install) $(TEE_ALL_MODULES.$(module_id).INSTALLED)
$(TEE_ALL_MODULES.$(module_id).INSTALLED): $(TEE_ALL_MODULES.$(module_id).$(TEE_MODE).BUILT)
	@echo Copying: $@
	$(hide) mkdir -p $(dir $@)
	$(hide) cp -f $(dir $<)$(notdir $@) $@

endif

ifeq (dump,dump)
TEE_dumpvar_log := $(TRUSTZONE_OUTPUT_PATH)/dump/$(module_id).log
$(TEE_dumpvar_log): DUMPVAR_VALUE := $(DUMPVAR_VALUE)
$(TEE_dumpvar_log): DUMPVAR_VALUE += $(TEE_MODE).BUILT=$(TEE_ALL_MODULES.$(module_id).$(TEE_MODE).BUILT)
$(TEE_dumpvar_log): DUMPVAR_VALUE += $(TEE_MODE).LIB=$(TEE_ALL_MODULES.$(module_id).$(TEE_MODE).LIB)
$(TEE_dumpvar_log): DUMPVAR_VALUE += $(TEE_MODE).AXF=$(TEE_ALL_MODULES.$(module_id).$(TEE_MODE).AXF)
$(TEE_dumpvar_log): PRIVATE_$(TEE_MODE).REQUIRED := $(TEE_ALL_MODULES.$(module_id).$(TEE_MODE).REQUIRED)
$(TEE_dumpvar_log): PRIVATE_$(TEE_MODE).HAL_LIBS := $(TEE_ALL_MODULES.$(module_id).$(TEE_MODE).HAL_LIBS)
  ifeq ($(TEE_MODE),$(strip $(TEE_INSTALL_MODE)))
$(TEE_dumpvar_log): DUMPVAR_VALUE += OUTPUT_NAME=$(strip $(OUTPUT_NAME))
    ifneq ($(strip $(DRIVER_UUID)),)
$(TEE_dumpvar_log): DUMPVAR_VALUE += DRIVER_UUID=$(strip $(DRIVER_UUID))
$(TEE_dumpvar_log): DUMPVAR_VALUE += DRIVER_MEMTYPE=$(strip $(DRIVER_MEMTYPE))
$(TEE_dumpvar_log): DUMPVAR_VALUE += DRIVER_NO_OF_THREADS=$(strip $(DRIVER_NO_OF_THREADS))
$(TEE_dumpvar_log): DUMPVAR_VALUE += DRIVER_SERVICE_TYPE=$(strip $(DRIVER_SERVICE_TYPE))
$(TEE_dumpvar_log): DUMPVAR_VALUE += DRIVER_KEYFILE=$(strip $(DRIVER_KEYFILE))
$(TEE_dumpvar_log): DUMPVAR_VALUE += DRIVER_FLAGS=$(strip $(DRIVER_FLAGS))
$(TEE_dumpvar_log): DUMPVAR_VALUE += DRIVER_VENDOR_ID=$(strip $(DRIVER_VENDOR_ID))
$(TEE_dumpvar_log): DUMPVAR_VALUE += DRIVER_NUMBER=$(strip $(DRIVER_NUMBER))
      ifneq ($(strip $(DRIVER_ID)),)
        ifeq ($(findstring <<16|,$(strip $(DRIVER_ID))),)
$(TEE_dumpvar_log): DUMPVAR_VALUE += DRIVER_ID=$(strip $(DRIVER_ID))
        endif
      endif
$(TEE_dumpvar_log): DUMPVAR_VALUE += DRIVER_INTERFACE_VERSION_MAJOR=$(strip $(DRIVER_INTERFACE_VERSION_MAJOR))
$(TEE_dumpvar_log): DUMPVAR_VALUE += DRIVER_INTERFACE_VERSION_MINOR=$(strip $(DRIVER_INTERFACE_VERSION_MINOR))
$(TEE_dumpvar_log): DUMPVAR_VALUE += DRIVER_INTERFACE_VERSION=$(strip $(DRIVER_INTERFACE_VERSION))
$(TEE_dumpvar_log): DUMPVAR_VALUE += BUILD_DRIVER_LIBRARY_ONLY=$(strip $(BUILD_DRIVER_LIBRARY_ONLY))
    else ifneq ($(strip $(TRUSTLET_UUID)),)
$(TEE_dumpvar_log): DUMPVAR_VALUE += TRUSTLET_UUID=$(strip $(TRUSTLET_UUID))
$(TEE_dumpvar_log): DUMPVAR_VALUE += TRUSTLET_MEMTYPE=$(strip $(TRUSTLET_MEMTYPE))
$(TEE_dumpvar_log): DUMPVAR_VALUE += TRUSTLET_NO_OF_THREADS=$(strip $(TRUSTLET_NO_OF_THREADS))
$(TEE_dumpvar_log): DUMPVAR_VALUE += TRUSTLET_SERVICE_TYPE=$(strip $(TRUSTLET_SERVICE_TYPE))
$(TEE_dumpvar_log): DUMPVAR_VALUE += TRUSTLET_KEYFILE=$(strip $(TRUSTLET_KEYFILE))
$(TEE_dumpvar_log): DUMPVAR_VALUE += TRUSTLET_FLAGS=$(strip $(TRUSTLET_FLAGS))
$(TEE_dumpvar_log): DUMPVAR_VALUE += TRUSTLET_INSTANCES=$(strip $(TRUSTLET_INSTANCES))
$(TEE_dumpvar_log): DUMPVAR_VALUE += TRUSTLET_MOBICONFIG_KEY=$(strip $(TRUSTLET_MOBICONFIG_KEY))
$(TEE_dumpvar_log): DUMPVAR_VALUE += TRUSTLET_MOBICONFIG_KID=$(strip $(TRUSTLET_MOBICONFIG_KID))
$(TEE_dumpvar_log): DUMPVAR_VALUE += TRUSTLET_MOBICONFIG_USE=$(strip $(TRUSTLET_MOBICONFIG_USE))
$(TEE_dumpvar_log): DUMPVAR_VALUE += BUILD_TRUSTLET_LIBRARY_ONLY=$(strip $(BUILD_TRUSTLET_LIBRARY_ONLY))
$(TEE_dumpvar_log): DUMPVAR_VALUE += GP_ENTRYPOINTS=$(strip $(GP_ENTRYPOINTS))
$(TEE_dumpvar_log): DUMPVAR_VALUE += TA_INTERFACE_VERSION=$(strip $(TA_INTERFACE_VERSION))
$(TEE_dumpvar_log): DUMPVAR_VALUE += TA_KEYFILE=$(strip $(TA_KEYFILE))
$(TEE_dumpvar_log): DUMPVAR_VALUE += TA_ROLLBACK_PROTECTED=$(strip $(TA_ROLLBACK_PROTECTED))
    endif
$(TEE_dumpvar_log): DUMPVAR_VALUE += HW_FLOATING_POINT=$(strip $(HW_FLOATING_POINT))
$(TEE_dumpvar_log): DUMPVAR_VALUE += TBASE_API_LEVEL=$(strip $(TBASE_API_LEVEL))
$(TEE_dumpvar_log): DUMPVAR_VALUE += HEAP_SIZE_INIT=$(strip $(HEAP_SIZE_INIT))
$(TEE_dumpvar_log): DUMPVAR_VALUE += HEAP_SIZE_MAX=$(strip $(HEAP_SIZE_MAX))
$(TEE_dumpvar_log): DUMPVAR_VALUE += MAKEFILE=$(TEE_ALL_MODULES.$(module_id).MAKEFILE)
$(TEE_dumpvar_log): DUMPVAR_VALUE += PATH=$(TEE_ALL_MODULES.$(module_id).PATH)
$(TEE_dumpvar_log): DUMPVAR_VALUE += CLASS=$(TEE_ALL_MODULES.$(module_id).CLASS)
$(TEE_dumpvar_log): PRIVATE_INSTALLED := $(TEE_ALL_MODULES.$(module_id).INSTALLED)
TEE_modules_to_check := $(TEE_modules_to_check) $(TEE_dumpvar_log)
$(TEE_dumpvar_log): $(TEE_ALL_MODULES.$(module_id).MAKEFILE) $(TEE_BASE_RULES)
	@echo TEE Dump: $@
	@mkdir -p $(dir $@)
	@rm -f $@
	@$(foreach v,$(DUMPVAR_VALUE),echo $(v) >>$@;)
	@$(foreach v,$(TEE_BUILD_MODE),echo $(v).REQUIRED=$(PRIVATE_$(v).REQUIRED) >>$@;)
	@$(foreach v,$(TEE_BUILD_MODE),echo $(v).HAL_LIBS=$(PRIVATE_$(v).HAL_LIBS) >>$@;)
	@echo INSTALLED=$(PRIVATE_INSTALLED) >>$@

  endif
endif
