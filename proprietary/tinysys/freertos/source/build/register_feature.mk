###########################################################
## Template for configuring FEATURE resources
###########################################################

# FEATURE_CONFIG is mandatory
FEATURE_CONFIG := $(strip $(FEATURE_CONFIG))

ifeq (,$(FEATURE_CONFIG))
  $(error $(TINYSYS_SCP): FEATURE_CONFIG is mandatory)
endif

ifeq (,$(FEATURE_NAME))
  $(error $(TINYSYS_SCP): FEATURE_NAME is mandatory (config: $(FEATURE_CONFIG)))
endif

# Feature names must be unique
ifneq (,$(filter $(ALL_FEATURES),$(FEATURE_NAME)))
  $(error $(TINYSYS_SCP): Feature $(FEATURE_NAME) is already defined)
endif

# Proceed only if FEATURE_CONFIG is yes
ifeq (yes,$($(FEATURE_CONFIG)))
$(PROCESSOR).$(FEATURE_NAME).CONFIG   := $(strip $(FEATURE_CONFIG))
$(PROCESSOR).$(FEATURE_NAME).INCLUDES := $(strip $(FEATURE_INCLUDES))
$(PROCESSOR).$(FEATURE_NAME).C_FILES  := $(strip $(FEATURE_C_FILES))
$(PROCESSOR).$(FEATURE_NAME).S_FILES  := $(strip $(FEATURE_S_FILES))
$(PROCESSOR).$(FEATURE_NAME).LIBFLAGS := $(strip $(FEATURE_LIBFLAGS))
$(PROCESSOR).$(FEATURE_NAME).CFLAGS   := $(strip $(FEATURE_CFLAGS))

$(PROCESSOR).ALL_FEATURES := \
  $(strip $($(PROCESSOR).ALL_FEATURES) $(FEATURE_NAME))
$(PROCESSOR).FEATURE_CONFIG_NAMES += $(FEATURE_CONFIG)

INCLUDES += $($(PROCESSOR).$(FEATURE_NAME).INCLUDES)
C_FILES  += $(strip $(FEATURE_SCP_C_FILES))
S_FILES  += $(strip $(FEATURE_SCP_S_FILES))
CFLAGS   += $(strip $(FEATURE_SCP_CFLAGS))
LDFLAGS  += $(strip $(FEATURE_SCP_LDFLAGS))
else
ifeq ($(V),1)
  $(info $(TINYSYS_DO): Feature $(FEATURE_NAME) for $(PROCESSOR) is disabled)
endif
endif # $(FEATURE_CONFIG) is yes
