DEPS += $(lastword $(MAKEFILE_LIST))

MY_DO_BUILT_DIR                  := $($(PROCESSOR).DO_BUILT_DIR)/$(do)
$(PROCESSOR).$(do).STEM          := \
  $(MY_DO_BUILT_DIR)/$(TINYSYS_DO)-$(PROCESSOR)-$(do)
$(PROCESSOR).$(do).ELF           := $($(PROCESSOR).$(do).STEM).elf
$(PROCESSOR).$(do).ELF_STRIPPED  := $($(PROCESSOR).$(do).STEM)-stripped.elf
$(PROCESSOR).$(do).BIN           := $($(PROCESSOR).$(do).STEM).bin
$(PROCESSOR).$(do).DO            := $($(PROCESSOR).$(do).STEM).do
$(PROCESSOR).$(do).DO_NO_DO_HDR  := $($(PROCESSOR).$(do).STEM)-no-do-header.do
$(PROCESSOR).$(do).DO_POSTPROCESS_STAGE1 := \
  $($(PROCESSOR).$(do).DO)-01_resolve_syms_tinysys
$(PROCESSOR).$(do).DO_POSTPROCESS_STAGE2 := \
  $($(PROCESSOR).$(do).DO)-02_resolve_syms_do
$(PROCESSOR).$(do).DO_POSTPROCESS_STAGE3 := \
  $($(PROCESSOR).$(do).DO)-03_fix_rel
$(PROCESSOR).$(do).DO_POSTPROCESS_STAGE4 := \
  $($(PROCESSOR).$(do).DO)-04_do_header
$(PROCESSOR).$(do).MAP          := $($(PROCESSOR).$(do).STEM).map
$(PROCESSOR).ALL_DO_BINS        := \
  $(strip $($(PROCESSOR).ALL_DO_BINS) $($(PROCESSOR).$(do).BIN))
$(PROCESSOR).ALL_DOS_BUILT      := \
  $(strip $($(PROCESSOR).ALL_DOS_BUILT) $($(PROCESSOR).$(do).DO))

# Ensure that all features registered in this DO is defined.
# Also record the used features in $(PROCESSOR).ALL_DO_FEATURES
INVALID_FEATURES :=
$(foreach f,$($(PROCESSOR).$(do).FEATURES), \
  $(if $(filter $(f),$($(PROCESSOR).ALL_FEATURES)), \
    $(eval $(PROCESSOR).ALL_DO_FEATURES := $(sort $($(PROCESSOR).ALL_DO_FEATURES) $(f))), \
    $(eval INVALID_FEATURES := $(INVALID_FEATURES) $(f))) \
)

ifneq (,$(strip $(INVALID_FEATURES)))
  $(error $(TINYSYS_DO): $(do): Unknown feature: $(INVALID_FEATURES) )
endif

# Initialize feature related rules for DO
$(foreach feature,$($(PROCESSOR).$(do).FEATURES), \
  $(eval include $(BUILD_DIR)/feature.mk) \
)

########################################
# ELF and BIN targets
########################################
MY_LIBFLAGS_SEARCH_PATH := $(patsubst -L%,%,$(filter -L%,$($(PROCESSOR).$(do).LIBFLAGS)))
MY_LIBFLAGS_SEARCH_NAME := $(patsubst -l%,lib%.a,$(filter -l%,$($(PROCESSOR).$(do).LIBFLAGS)))
MY_LIBFLAGS_SEARCH_FILE := $(wildcard $(foreach p,$(MY_LIBFLAGS_SEARCH_PATH),$(foreach n,$(MY_LIBFLAGS_SEARCH_NAME),$(p)/$(n))))
$($(PROCESSOR).$(do).ELF): PRIVATE_BUILT_DIR := $(MY_DO_BUILT_DIR)
$($(PROCESSOR).$(do).ELF): PRIVATE_LDFLAGS := \
  $(DO_COMMON_LDFLAGS) $($(PROCESSOR).$(do).LDFLAGS)
$($(PROCESSOR).$(do).ELF): PRIVATE_LIBFLAGS := $($(PROCESSOR).$(do).LIBFLAGS)
$($(PROCESSOR).$(do).ELF): PRIVATE_OBJS := \
  $($(PROCESSOR).$(do).ALL_FEATURE_OBJS) $($(PROCESSOR).DO_GENERIC_OBJS)
$($(PROCESSOR).$(do).ELF): PRIVATE_MAP_FILE := $($(PROCESSOR).$(do).MAP)
$($(PROCESSOR).$(do).ELF): \
  $($(PROCESSOR).$(do).ALL_FEATURE_OBJS) $($(PROCESSOR).DO_GENERIC_OBJS)
$($(PROCESSOR).$(do).ELF): $(MY_LIBFLAGS_SEARCH_FILE)
$($(PROCESSOR).$(do).ELF): $(DEPS)
	@echo '$(TINYSYS_DO): ELF     $@'
	@mkdir -p $(PRIVATE_BUILT_DIR)
	$(hide)$(CC) $(call normalize-includes,$(DO_COMMON_INCLUDES)) $(PRIVATE_LDFLAGS) $(PRIVATE_OBJS) -Wl,-Map=$(PRIVATE_MAP_FILE) -o $@ -Wl,--start-group $(PRIVATE_LIBFLAGS) -Wl,--end-group

$($(PROCESSOR).$(do).ELF_STRIPPED): PRIVATE_BUILT_DIR := $(MY_DO_BUILT_DIR)
$($(PROCESSOR).$(do).ELF_STRIPPED): $($(PROCESSOR).$(do).ELF) $(DEPS)
	@echo '$(TINYSYS_DO): STRIP   $@'
	@mkdir -p $(PRIVATE_BUILT_DIR)
	$(hide)$(STRIP) $(DO_STRIP_OPTIONS) $< -o $@

$($(PROCESSOR).$(do).BIN): PRIVATE_BUILT_DIR := $(MY_DO_BUILT_DIR)
$($(PROCESSOR).$(do).BIN): $($(PROCESSOR).$(do).ELF_STRIPPED) $(DEPS)
	@echo '$(TINYSYS_DO): OBJCOPY $@'
	@mkdir -p $(PRIVATE_BUILT_DIR)
	$(hide)$(OBJCOPY) $(DO_OBJCOPY_OPTIONS) $< $@

########################################
# DO targets
########################################
$($(PROCESSOR).$(do).DO): PRIVATE_DO_NAME := $(do)
$($(PROCESSOR).$(do).DO): \
  PRIVATE_DO_FEATURES := $($(PROCESSOR).$(do).FEATURES)
$($(PROCESSOR).$(do).DO): \
  $($(PROCESSOR).$(do).DO_NO_DO_HDR) $(MKDOHEADER) \
  $(DEPS)
	@echo '$(TINYSYS_DO): MKDOHDR $@'
	$(hide)$(MKDOHEADER) $(PRIVATE_DO_NAME) $< $@ $(PRIVATE_DO_FEATURES)

$($(PROCESSOR).$(do).DO_NO_DO_HDR): $($(PROCESSOR).$(do).DO_POSTPROCESS_STAGE4) $(DEPS)
	@echo '$(TINYSYS_DO): OBJCOPY $@'
	$(hide)$(OBJCOPY) -O binary $< $@

$($(PROCESSOR).$(do).DO_POSTPROCESS_STAGE4): \
  PRIVATE_DO_STAGE3 := $($(PROCESSOR).$(do).DO_POSTPROCESS_STAGE3)
$($(PROCESSOR).$(do).DO_POSTPROCESS_STAGE4): \
  PRIVATE_SCP_ELF := $($(PROCESSOR).ELF_FILE)
$($(PROCESSOR).$(do).DO_POSTPROCESS_STAGE4): \
  $($(PROCESSOR).$(do).DO_POSTPROCESS_STAGE3) \
  $(DO_ELFTOOLS_DEPS) \
  $(DEPS) \
  $(DO_ELFTOOLS_DIR)/scripts/update_do_header.py
	@echo '$(TINYSYS_DO): DOSTAGE4 $@'
	$(hide)$(DO_PYTHON_ENV) $(DO_ELFTOOLS_DIR)/scripts/update_do_header.py --do-file $(PRIVATE_DO_STAGE3) --tinysys-file $(PRIVATE_SCP_ELF) --output-file $@

$($(PROCESSOR).$(do).DO_POSTPROCESS_STAGE3): \
  PRIVATE_DO_STAGE2 := $($(PROCESSOR).$(do).DO_POSTPROCESS_STAGE2)
$($(PROCESSOR).$(do).DO_POSTPROCESS_STAGE3): \
  PRIVATE_SCP_ELF := $($(PROCESSOR).ELF_FILE)
$($(PROCESSOR).$(do).DO_POSTPROCESS_STAGE3): \
  $($(PROCESSOR).$(do).DO_POSTPROCESS_STAGE2) \
  $(DO_ELFTOOLS_DEPS) \
  $(DEPS) \
  $(DO_ELFTOOLS_DIR)/scripts/fixup_relocations.py
	@echo '$(TINYSYS_DO): DOSTAGE3 $@'
	$(hide)$(DO_PYTHON_ENV) $(DO_ELFTOOLS_DIR)/scripts/fixup_relocations.py --do-file $(PRIVATE_DO_STAGE2) --tinysys-file $(PRIVATE_SCP_ELF) --output-file $@

$($(PROCESSOR).$(do).DO_POSTPROCESS_STAGE2): \
  PRIVATE_DO_STAGE1 := $($(PROCESSOR).$(do).DO_POSTPROCESS_STAGE1)
$($(PROCESSOR).$(do).DO_POSTPROCESS_STAGE2): \
  PRIVATE_SCP_ELF := $($(PROCESSOR).ELF_FILE)
$($(PROCESSOR).$(do).DO_POSTPROCESS_STAGE2): \
  $($(PROCESSOR).$(do).DO_POSTPROCESS_STAGE1) \
  $(DO_ELFTOOLS_DEPS) \
  $(DEPS) \
  $(DO_ELFTOOLS_DIR)/scripts/resolve_do_rest_symbols.py
	@echo '$(TINYSYS_DO): DOSTAGE2 $@'
	$(hide)$(DO_PYTHON_ENV) $(DO_ELFTOOLS_DIR)/scripts/resolve_do_rest_symbols.py --do-file $(PRIVATE_DO_STAGE1) --tinysys-file $(PRIVATE_SCP_ELF) --output-file $@

$($(PROCESSOR).$(do).DO_POSTPROCESS_STAGE1): \
  PRIVATE_ELF_STRIPPED := $($(PROCESSOR).$(do).ELF_STRIPPED)
$($(PROCESSOR).$(do).DO_POSTPROCESS_STAGE1): \
  PRIVATE_SCP_ELF := $($(PROCESSOR).ELF_FILE)
$($(PROCESSOR).$(do).DO_POSTPROCESS_STAGE1): \
  $($(PROCESSOR).$(do).ELF_STRIPPED) $($(PROCESSOR).ELF_FILE) \
  $(DO_ELFTOOLS_DEPS) \
  $(DEPS) \
  $(DO_ELFTOOLS_DIR)/scripts/resolve_tinysys_scp_symbols.py
	@echo '$(TINYSYS_DO): DOSTAGE1 $@'
	$(hide)$(DO_PYTHON_ENV) $(DO_ELFTOOLS_DIR)/scripts/resolve_tinysys_scp_symbols.py --do-file $(PRIVATE_ELF_STRIPPED) --tinysys-file $(PRIVATE_SCP_ELF) --output-file $@
