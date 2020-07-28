###########################################################
## Generic build configurations
###########################################################
include $(BUILD_DIR)/clear_vars.mk

###########################################################
## Create processor-based environment and targets
###########################################################
PROCESSOR_DIR                  := $(BASE_DIR)/$(PROCESSOR)
PLATFORM_BASE_DIR              := $(PROCESSOR_DIR)/$(PLATFORM)
PLATFORM_DIR                   := $(PLATFORM_BASE_DIR)/platform
PROJECT_DIR                    := $(PLATFORM_BASE_DIR)/$(PROJECT)
FEATURE_CONFIG_DIR             := $(PLATFORM_DIR)/feature_config
MY_BUILT_DIR                   := $(BUILT_DIR)/$(PROCESSOR)
DRIVERS_PLATFORM_DIR           := $(DRIVERS_DIR)/$(PROCESSOR)/$(PLATFORM)
GENERATED_DIR                  := $(MY_BUILT_DIR)/generated
GENERATED_INCLUDE_DIR          := $(GENERATED_DIR)/include
GENERATED_SYSTEM_LD            := $(GENERATED_DIR)/system.ld
GENERATED_SYSTEM_LD_TEMP        := $(GENERATED_SYSTEM_LD).temp
SYSTEM_LD_TEMPLATE             := $(PLATFORM_DIR)/system.ld.template
PROJECT_LD                     := $(strip $(wildcard $(PROJECT_DIR)/project.ld))
$(PROCESSOR).TINYSYS_CONFIG_H  := $(GENERATED_INCLUDE_DIR)/tinysys_config.h

PLATFORM_MK                    := $(PLATFORM_DIR)/platform.mk
ifeq ($(wildcard $(PLATFORM_MK)),)
  $(error $(TINYSYS_SCP): $(PLATFORM_MK) is missing)
endif

# Reset any previously defined SCP options before including my own definitions
$(foreach v,$(SCP_PREVIOUS_CFG_OPTS), \
  $(eval $(v) := ) \
)
scp_old_cfgs := $(.VARIABLES)

include $(PLATFORM_MK)

INCLUDES += $(GENERATED_INCLUDE_DIR)

ifneq ($(filter CM4_%,$(PROCESSOR)),)
  include $(BUILD_DIR)/config_cm4.mk
else
  ifneq ($(filter HIFI3_%,$(PROCESSOR)),)
    include $(BUILD_DIR)/config_hifi3.mk
  else
    $(error $(TINYSYS_SCP): unsupported processor: $(PROCESSOR))
  endif
endif

ifneq (no,$(BUILD_FROM_SOURCE))
scp_new_cfgs := $(.VARIABLES)
SCP_PREVIOUS_CFG_OPTS := \
  $(filter CFG_%,$(filter-out $(scp_old_cfgs),$(scp_new_cfgs)))

# Set dependency
DEPS += $(sort $(filter-out %.d,$(MAKEFILE_LIST)))

$(OBJS): $($(PROCESSOR).TINYSYS_CONFIG_H)

# Stash the list of configuration names and values to generate config header
CONFIG_MK_FILES := \
  $(PLATFORM_MK) \
  $(wildcard $(PLATFORM_DIR)/dos.mk) \
  $(wildcard $(PROJECT_DIR)/ProjectConfig.mk)
$(call stash_config_options,$(CONFIG_MK_FILES))

INCLUDES := $(call normalize-includes,$(INCLUDES))

###########################################################
## Generic build targets
###########################################################
$(C_OBJS): PRIVATE_CC := $(CC)
$(C_OBJS): PRIVATE_CFLAGS := $(CFLAGS)
$(C_OBJS): PRIVATE_INCLUDES := $(INCLUDES)
$(C_OBJS): $(MY_BUILT_DIR)/%.o: %.c
	$(compile-c-or-s-to-o)

$(S_OBJS): PRIVATE_CC := $(CC)
$(S_OBJS): PRIVATE_CFLAGS := $(CFLAGS)
$(S_OBJS): PRIVATE_INCLUDES := $(INCLUDES)
$(S_OBJS): $(MY_BUILT_DIR)/%.o: %.S
	$(compile-c-or-s-to-o)

$(CPP_OBJS): PRIVATE_CC := $(CPP)
$(CPP_OBJS): PRIVATE_CFLAGS := $(CFLAGS)
$(CPP_OBJS): PRIVATE_INCLUDES := $(INCLUDES)
$(CPP_OBJS): $(MY_BUILT_DIR)/%.o: %.cpp
	$(compile-c-or-s-to-o)

$(OBJS): $(DEPS)
-include $(OBJS:.o=.d)
endif # BUILD_FROM_SOURCE

# Generate header file that contains all config options and its values
.PHONY: configheader
configheader: $($(PROCESSOR).TINYSYS_CONFIG_H)

$($(PROCESSOR).TINYSYS_CONFIG_H): PRIVATE_PROCESSOR := $(PROCESSOR)
# Let target header file depends on FORCE to ensure that for each make the
# config option updater mechanism kicks off. Target header file will not
# be updated if config option are not changed.
$($(PROCESSOR).TINYSYS_CONFIG_H): FORCE
	$(call gen-tinysys-header,__TINYSYS_CONFIG_H, $($(PRIVATE_PROCESSOR).CONFIG_OPTIONS), $(TINYSYS_SCP))
