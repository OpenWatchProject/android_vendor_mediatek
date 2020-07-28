###########################################################
## Template for configuring Dynamic Object resources.
###########################################################
# DO_CONFIG is mandatory
DO_CONFIG := $(strip $(DO_CONFIG))

DO_ID_MAX_LENGTH := 32

ifeq (,$(DO_CONFIG))
  $(error $(TINYSYS_DO): DO_CONFIG is mandatory)
endif

ifeq (,$(DO_NAME))
  $(error $(TINYSYS_DO): DO_NAME is mandatory (config: $(DO_CONFIG)))
endif

# DO name must be unique
ifneq (,$(filter $(ALL_DOS),$(DO_NAME)))
  $(error $(TINYSYS_DO): Feature $(DO_NAME) is already defined)
endif

DO_ID ?= $(DO_NAME)_ID

ifeq (yes,$(shell [ $$(expr length "$(DO_ID)") -gt $(DO_ID_MAX_LENGTH) ] && echo yes))
  $(error $(TINYSYS_DO): Length of DO ID "$(DO_ID)" ($(shell expr length "$(DO_ID)")) exceeds maximum length $(DO_ID_MAX_LENGTH))
endif

# Proceed only if DO_CONFIG is yes
ifeq (yes,$($(DO_CONFIG)))
$(PROCESSOR).$(DO_NAME).DO_ID    := $(strip $(DO_ID))
$(PROCESSOR).$(DO_NAME).INCLUDES := $(strip $(DO_INCLUDES))
$(PROCESSOR).$(DO_NAME).C_FILES  := $(strip $(DO_C_FILES))
$(PROCESSOR).$(DO_NAME).S_FILES  := $(strip $(DO_S_FILES))
$(PROCESSOR).$(DO_NAME).LDFLAGS  := $(strip $(DO_LDFLAGS))
$(PROCESSOR).$(DO_NAME).CFLAGS   := $(strip $(DO_CFLAGS))
$(PROCESSOR).$(DO_NAME).FEATURES := $(strip $(DO_FEATURES))

$(PROCESSOR).DO_CONFIG_NAMES += $(DO_CONFIG)

# Check if all features are enabled, and collect LIBFLAGS
$(PROCESSOR).$(DO_NAME).LIBFLAGS :=
$(foreach f,$(DO_FEATURES), \
  $(if $(filter yes,$($($(PROCESSOR).$(f).CONFIG))), \
    $(eval $(PROCESSOR).$(DO_NAME).LIBFLAGS += $($(PROCESSOR).$(f).LIBFLAGS)), \
    $(error $(TINYSYS_DO): Feature $(f), required by DO $(DO_NAME), must be enabled) \
  ) \
)

$(PROCESSOR).ALL_DOS := $(strip $($(PROCESSOR).ALL_DOS) $(DO_NAME))
endif # $(DO_CONFIG) is yes
