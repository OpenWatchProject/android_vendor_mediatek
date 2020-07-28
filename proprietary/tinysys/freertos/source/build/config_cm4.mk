###########################################################
## CM4 specific build rules
###########################################################
$(PROCESSOR).TINYSYS_BIN_BUILT := \
  $(MY_BUILT_DIR)/$(TINYSYS_SCP)-$(PROCESSOR).bin

MY_BIN_STEM              := $(basename $($(PROCESSOR).TINYSYS_BIN_BUILT))
$(PROCESSOR).BIN_NH      := $(MY_BIN_STEM)-no-mtk-header.bin
$(PROCESSOR).BIN_DRAM_NH := $(MY_BIN_STEM)_DRAM-no-mtk-header.bin
$(PROCESSOR).ELF_FILE    := $(MY_BIN_STEM).elf
$(PROCESSOR).MAP_FILE    := $(MY_BIN_STEM).map
$(PROCESSOR).HEX_FILE    := $(MY_BIN_STEM).hex
$(PROCESSOR).IMG_HDR_CFG := $(MY_BUILT_DIR)/img_hdr_$(notdir $(MY_BIN_STEM)).cfg
$(PROCESSOR)_DRAM.IMG_HDR_CFG := $(MY_BUILT_DIR)/img_hdr_$(notdir $(MY_BIN_STEM))_dram.cfg
$(PROCESSOR)_DRAM.TINYSYS_BIN_BUILT := \
  $(MY_BUILT_DIR)/$(TINYSYS_SCP)-$(PROCESSOR)_DRAM.bin

# List of config names of DO and features for this processor. Since those names
# are not processor-bound, they are unintentionally inherited by the next
# processor. They need to be reset before processing the next processor.
$(PROCESSOR).DO_CONFIG_NAMES      :=
$(PROCESSOR).FEATURE_CONFIG_NAMES :=

ALL_SCP_BINS := $(ALL_SCP_BINS) $($(PROCESSOR).TINYSYS_BIN_BUILT)
ALL_SCP_DRAM_BINS := $(ALL_SCP_DRAM_BINS) $($(PROCESSOR)_DRAM.TINYSYS_BIN_BUILT)

include $(BUILD_DIR)/env_cm4.mk
include $(BUILD_DIR)/loader.mk

ifeq (,$(PROJECT_LD))
  ifeq (true,$(MANDATORY_PROJECT_LD))
    $(error $(TINYSYS_SCP): project-specific ld file is required in multi-processor environment)
  endif
else
  GENERATED_PROJECT_LD := $(GENERATED_DIR)/project.ld
  LDFLAGS  += -Wl,-T$(GENERATED_PROJECT_LD)
endif

PLATFORM_SYSTEM_LD := $(GENERATED_SYSTEM_LD)
# Use platform-specific system.ld if available when DO is disabled
ifneq (,$(wildcard $(PLATFORM_DIR)/system.ld))
  ifneq (yes,$(CFG_DO_ENABLED))
    PLATFORM_SYSTEM_LD := $(PLATFORM_DIR)/system.ld
  else
    $(info $(TINYSYS_SCP): Ignored $(PLATFORM_DIR)/system.ld since $(PROCESSOR) enables DO support)
  endif
endif

LDFLAGS  += -Wl,-T$(PLATFORM_SYSTEM_LD)

# Include project-specific files only when available
ifneq ($(wildcard $(PROJECT_DIR)/inc),)
  INCLUDES += $(PROJECT_DIR)/inc
endif

# Set dependency
DO_ELFTOOLS_DEPS := $(if $(wildcard $(DO_ELFTOOLS_DIR)),$(shell find $(DO_ELFTOOLS_DIR) -type f))

include $(BUILD_DIR)/do_binary.mk

ifneq ($(wildcard $(PROJECT_DIR)/src/project.c),)
  C_FILES += $(wildcard $(PROJECT_DIR)/src/project.c)
endif

C_FILES   := $(patsubst ./%,%,$(C_FILES))
CPP_FILES := $(patsubst ./%,%,$(CPP_FILES))
S_FILES   := $(patsubst ./%,%,$(S_FILES))
C_OBJS    := $(sort $(C_FILES:%.c=$(MY_BUILT_DIR)/%.o))
CPP_OBJS  := $(sort $(CPP_FILES:%.cpp=$(MY_BUILT_DIR)/%.o))
S_OBJS    := $(sort $(S_FILES:%.S=$(MY_BUILT_DIR)/%.o))

# Filter out features not required by any DO, and add the C files of them
# to the SCP build target objects
FEATURES_IN_SCP := \
  $(filter-out $($(PROCESSOR).ALL_DO_FEATURES),$($(PROCESSOR).ALL_FEATURES))
ifeq (1,$(V))
  $(info $(TINYSYS_DO): Features built in $(PROCESSOR) SCP instead of DO: $(if $(FEATURES_IN_SCP),$(FEATURES_IN_SCP),none))
endif

$(foreach f,$(FEATURES_IN_SCP), \
  $(eval C_OBJS += \
    $(sort $($(PROCESSOR).$(f).C_FILES:%.c=$(MY_BUILT_DIR)/%.o))) \
  $(eval S_OBJS += \
    $(sort $($(PROCESSOR).$(f).S_FILES:%.S=$(MY_BUILT_DIR)/%.o))) \
  $(eval LIBFLAGS += $($(PROCESSOR).$(f).LIBFLAGS)) \
  $(eval CFLAGS += $($(PROCESSOR).$(f).CFLAGS)) \
)

OBJS += $(sort $(C_OBJS) $(S_OBJS) $(CPP_OBJS))

# Reset DO and FEATURE configs to prevent from propagating the value to next
# processor
$(foreach v,$($(PROCESSOR).DO_CONFIG_NAMES) $($(PROCESSOR).FEATURE_CONFIG_NAMES), $(eval $(v) :=))

CFLAGS += -include $($(PROCESSOR).TINYSYS_CONFIG_H)

INCLUDES := $(call normalize-includes,$(INCLUDES))

DEPS += $(sort $(filter-out %.d,$(MAKEFILE_LIST)))

###########################################################
## Create processor-based build targets
###########################################################
$($(PROCESSOR).TINYSYS_BIN_BUILT): PRIVATE_PROCESSOR:= $(PROCESSOR)
$($(PROCESSOR).TINYSYS_BIN_BUILT): PRIVATE_BIN_NH := $($(PROCESSOR).BIN_NH)
$($(PROCESSOR).TINYSYS_BIN_BUILT): PRIVATE_HEX_FILE := $($(PROCESSOR).HEX_FILE)
$($(PROCESSOR).TINYSYS_BIN_BUILT): PRIVATE_BUILT_DIR := $(MY_BUILT_DIR)
$($(PROCESSOR).TINYSYS_BIN_BUILT): PRIVATE_IMG_HDR_CFG := $($(PROCESSOR).IMG_HDR_CFG)
$($(PROCESSOR).TINYSYS_BIN_BUILT): PRIVATE_ELF_FILE := $($(PROCESSOR).ELF_FILE)
$($(PROCESSOR).TINYSYS_BIN_BUILT): PRIVATE_PLATFORM_DIR := $(PLATFORM_DIR)
$($(PROCESSOR).TINYSYS_BIN_BUILT): PRIVATE_PROJECT_DIR := $(PROJECT_DIR)
$($(PROCESSOR).TINYSYS_BIN_BUILT): PRIVATE_BIN_DRAM_BUILT := $($(PROCESSOR)_DRAM.TINYSYS_BIN_BUILT)
$($(PROCESSOR).TINYSYS_BIN_BUILT): PRIVATE_BIN_DRAM_NH := $($(PROCESSOR).BIN_DRAM_NH)
$($(PROCESSOR).TINYSYS_BIN_BUILT): PRIVATE_DRAM_IMG_HDR_CFG := $($(PROCESSOR)_DRAM.IMG_HDR_CFG)
$($(PROCESSOR).TINYSYS_BIN_BUILT): PRIVATE_OBJCOPY := $(OBJCOPY)

$($(PROCESSOR).TINYSYS_BIN_BUILT): \
  $($(PROCESSOR).ELF_FILE) $($(PROCESSOR).IMG_HDR_CFG) | $(OBJSIZE) $(MKIMAGE)
$($(PROCESSOR).TINYSYS_BIN_BUILT): $($(PROCESSOR).IMG_HDR_CFG) $($(PROCESSOR)_DRAM.IMG_HDR_CFG) $(DEPS)
	@echo '$(TINYSYS_SCP): BIN   $@'
	$(hide)$(PRIVATE_OBJCOPY) -O ihex $(PRIVATE_ELF_FILE) $(PRIVATE_HEX_FILE)
	$(hide)$(PRIVATE_OBJCOPY) -O binary --remove-section .dram_region $(PRIVATE_ELF_FILE) $(PRIVATE_BIN_NH)
	-rm -f $(PRIVATE_BIN_DRAM_NH)
	-rm -f $(PRIVATE_BIN_DRAM_BUILT)
	$(hide)$(PRIVATE_OBJCOPY) -O binary --only-section=.dram_region $(PRIVATE_ELF_FILE) $(PRIVATE_BIN_DRAM_NH)
	@if [ -s '$(PRIVATE_BIN_DRAM_NH)' ]; then \
		echo "$(TINYSYS_SCP): enable cache support for $(PRIVATE_PROCESSOR)"; \
		$(hide)$(MKIMAGE) $(PRIVATE_BIN_DRAM_NH) $(PRIVATE_DRAM_IMG_HDR_CFG) > $(PRIVATE_BIN_DRAM_BUILT); \
	fi;
	$(hide)$(MKIMAGE) $(PRIVATE_BIN_NH) $(PRIVATE_IMG_HDR_CFG) > $@

.PHONY: $($(PROCESSOR)_DRAM.TINYSYS_BIN_BUILT)

ifeq (1,$(V))
	$(hide)$(SIZE) $(PRIVATE_ELF_FILE)
	$(hide)$(OBJSIZE) $(PRIVATE_BUILT_DIR)/$(RTOS_SRC_DIR)
	$(hide)$(OBJSIZE) $(PRIVATE_BUILT_DIR)/$(PRIVATE_PLATFORM_DIR)
	$(hide)$(OBJSIZE) $(PRIVATE_BUILT_DIR)/$(PRIVATE_PROJECT_DIR)
endif

MY_LIBFLAGS_SEARCH_PATH := $(patsubst -L%,%,$(filter -L%,$(LIBFLAGS)))
MY_LIBFLAGS_SEARCH_NAME := $(patsubst -l%,lib%.a,$(filter -l%,$(LIBFLAGS)))
MY_LIBFLAGS_SEARCH_FILE := $(wildcard $(foreach p,$(MY_LIBFLAGS_SEARCH_PATH),$(foreach n,$(MY_LIBFLAGS_SEARCH_NAME),$(p)/$(n))))
$($(PROCESSOR).ELF_FILE): PRIVATE_CC := $(CC)
$($(PROCESSOR).ELF_FILE): PRIVATE_PLATFORM := $(PLATFORM)
$($(PROCESSOR).ELF_FILE): PRIVATE_MAP_FILE := $($(PROCESSOR).MAP_FILE)
$($(PROCESSOR).ELF_FILE): PRIVATE_LDFLAGS := $(LDFLAGS)
$($(PROCESSOR).ELF_FILE): PRIVATE_LIBFLAGS := $(LIBFLAGS)
$($(PROCESSOR).ELF_FILE): PRIVATE_OBJS := $(OBJS)
$($(PROCESSOR).ELF_FILE): $(OBJS) $(PLATFORM_SYSTEM_LD)
$($(PROCESSOR).ELF_FILE): $(GENERATED_PROJECT_LD)
$($(PROCESSOR).ELF_FILE): $(MY_LIBFLAGS_SEARCH_FILE)
$($(PROCESSOR).ELF_FILE): $(DEPS)
	@echo '$(TINYSYS_SCP): ELF     $@'
	$(hide)$(PRIVATE_CC) $(PRIVATE_LDFLAGS) $(PRIVATE_OBJS) -Wl,-Map=$(PRIVATE_MAP_FILE) -o $@ -Wl,--start-group $(PRIVATE_LIBFLAGS) -Wl,--end-group
	@echo '$(TINYSYS_SCP): Memory Check'
	$(hide)PLATFORM=$(PRIVATE_PLATFORM) $(MCHECK) -d SCP $(SETTING) $(PRIVATE_MAP_FILE)

$(GENERATED_PROJECT_LD): PRIVATE_CNN_TO_SCP_BUF_SIZE := $(CFG_CNN_TO_SCP_BUF_SIZE)
$(GENERATED_PROJECT_LD): PRIVATE_SCP_TO_CNN_BUF_SIZE := $(CFG_SCP_TO_CNN_BUF_SIZE)
$(GENERATED_PROJECT_LD): PRIVATE_L1C_DRAM_ADDR := $(CFG_L1C_DRAM_ADDR)
$(GENERATED_PROJECT_LD): PRIVATE_L1C_DRAM_SIZE := $(CFG_L1C_DRAM_SIZE)
$(GENERATED_PROJECT_LD): PRIVATE_PROJECT_LD := $(PROJECT_LD)
$(GENERATED_PROJECT_LD): PRIVATE_CCPP := $(CC_PREPROCESSOR)
$(GENERATED_PROJECT_LD): PRIVATE_CAFLAG := $(CAFLAG)
$(GENERATED_PROJECT_LD): $(DEPS)
	$(hide)mkdir -p $(dir $@)
	$(PRIVATE_CCPP) $(PRIVATE_CAFLAG) $(PRIVATE_PROJECT_LD) -o $@.tmp
	$(hide)sed -i '/^#.*/ d' $@.tmp
	$(hide)sed -ir "s/@@CFG_CNN_TO_SCP_BUF_SIZE@@/$(strip $(PRIVATE_CNN_TO_SCP_BUF_SIZE))/g" $@.tmp
	$(hide)sed -ir "s/@@CFG_SCP_TO_CNN_BUF_SIZE@@/$(strip $(PRIVATE_SCP_TO_CNN_BUF_SIZE))/g" $@.tmp
	$(hide)sed -ir "s/@@CFG_L1C_DRAM_ADDR@@/$(strip $(PRIVATE_L1C_DRAM_ADDR))/g" $@.tmp
	$(hide)sed -ir "s/@@CFG_L1C_DRAM_SIZE@@/$(strip $(PRIVATE_L1C_DRAM_SIZE))/g" $@.tmp
	$(hide)if [ -f '$@' ]; then \
		if cmp -s '$@' '$(@).tmp'; then \
			rm '$(@).tmp'; \
			echo '$(TINYSYS_SCP): $(@) is update to date.'; \
		else \
			mv '$(@).tmp' '$@'; \
			echo '$(TINYSYS_SCP): Updated $@'; \
		fi; \
	else \
		mv '$(@).tmp' '$@'; \
		echo '$(TINYSYS_SCP): GEN     $@'; \
	fi


$(GENERATED_SYSTEM_LD): PRIVATE_SYSTEM_LD_SRC := $(GENERATED_SYSTEM_LD_TEMP)
$(GENERATED_SYSTEM_LD): $(GENERATED_SYSTEM_LD_TEMP)
$(GENERATED_SYSTEM_LD): PRIVATE_PROCESSOR := $(PROCESSOR)
$(GENERATED_SYSTEM_LD): PRIVATE_ELF_FILES := $(patsubst %.bin,%.elf,$($(PROCESSOR).ALL_DO_BINS))
$(GENERATED_SYSTEM_LD): PRIVATE_CNN_TO_SCP_BUF_SIZE := $(CFG_CNN_TO_SCP_BUF_SIZE)
$(GENERATED_SYSTEM_LD): PRIVATE_SCP_TO_CNN_BUF_SIZE := $(CFG_SCP_TO_CNN_BUF_SIZE)
$(GENERATED_SYSTEM_LD): $($(PROCESSOR).ALL_DO_BINS)
$(GENERATED_SYSTEM_LD): $(DO_ELFTOOLS_DEPS)
$(GENERATED_SYSTEM_LD): $(DEPS)
	$(hide)MAX_DO_SIZE=0; \
		for i in $(PRIVATE_ELF_FILES); do \
			sz=$$($(DO_PYTHON_ENV) $(DO_ELFTOOLS_DIR)/scripts/calculate_required_sram_size.py --do-file $${i}); \
			[ $${sz} -eq 0 ] && exit 1; \
			expr $${sz} \> $${MAX_DO_SIZE} > /dev/null && MAX_DO_SIZE=$${sz}; \
		done; \
		mkdir -p $(dir $@) ; \
		sed -r "s/@@_Cur_DO_WS_Size@@/$$(printf '0x%x' $${MAX_DO_SIZE})/g" $(PRIVATE_SYSTEM_LD_SRC) > $@.tmp
	$(hide)sed -ir "s/@@Keep_Symbols@@/$(strip $(call get_do_undefined_symbols,$(PRIVATE_PROCESSOR)))/g" $@.tmp
	$(hide)if [ -f '$@' ]; then \
		if cmp -s '$@' '$(@).tmp'; then \
			rm '$(@).tmp'; \
			echo '$(TINYSYS_SCP): $(@) is update to date.'; \
		else \
			mv '$(@).tmp' '$@'; \
			echo '$(TINYSYS_SCP): Updated $@'; \
		fi; \
	else \
		mv '$(@).tmp' '$@'; \
		echo '$(TINYSYS_SCP): GEN     $@'; \
	fi

$(GENERATED_SYSTEM_LD_TEMP): PRIVATE_CCPP := $(CC_PREPROCESSOR)
$(GENERATED_SYSTEM_LD_TEMP): PRIVATE_INCLUDES := $(INCLUDES)
$(GENERATED_SYSTEM_LD_TEMP): $(SYSTEM_LD_TEMPLATE) configheader
	$(hide)mkdir -p $(dir $@)
	$(PRIVATE_CCPP) $(PRIVATE_INCLUDES) $< -o $@
	$(hide)sed -i '/^#.*/ d' $@

$($(PROCESSOR).IMG_HDR_CFG): $(DEPS)
	$(call gen-image-header,$(TINYSYS_SCP))

$($(PROCESSOR)_DRAM.IMG_HDR_CFG): $(DEPS)
	$(call gen-image-header,$(TINYSYS_SCP))
