###########################################################
## Create processor-based environment and targets
###########################################################

BUILD_FROM_SOURCE := no

ifeq (yes,$(CFG_MTK_AUDIODSP_SUPPORT))
ifeq (yes,$(BUILD_WITH_XTENSA))
BUILD_FROM_SOURCE := yes

MY_BIN_STEM              := $(MY_BUILT_DIR)/$(PROCESSOR_LC)
$(PROCESSOR).ELF_FILE    := $(MY_BIN_STEM).elf
$(PROCESSOR).MAP_FILE    := $(MY_BIN_STEM).map

$(PROCESSOR).DRAM_TEXT   := $(MY_BIN_STEM)_dram.txt
$(PROCESSOR).DRAM_BIN_NH := $(MY_BIN_STEM)_dram_no_mtk_header.bin
$(PROCESSOR).DRAM_BIN    := $(MY_BIN_STEM)_dram.bin
$(PROCESSOR).DRAM_HDRCFG := \
  $(MY_BUILT_DIR)/img_hdr_$(notdir $(basename $($(PROCESSOR).DRAM_BIN))).cfg
$(PROCESSOR).IRAM_TEXT   := $(MY_BIN_STEM)_iram.txt
$(PROCESSOR).IRAM_BIN_NH := $(MY_BIN_STEM)_iram_no_mtk_header.bin
$(PROCESSOR).IRAM_BIN    := $(MY_BIN_STEM)_iram.bin
$(PROCESSOR).IRAM_HDRCFG := \
  $(MY_BUILT_DIR)/img_hdr_$(notdir $(basename $($(PROCESSOR).IRAM_BIN))).cfg
$(PROCESSOR).SRAM_TEXT   := $(MY_BIN_STEM)_sram.txt
$(PROCESSOR).SRAM_BIN_NH := $(MY_BIN_STEM)_sram_no_mtk_header.bin
$(PROCESSOR).SRAM_BIN    := $(MY_BIN_STEM)_sram.bin
$(PROCESSOR).SRAM_HDRCFG := \
  $(MY_BUILT_DIR)/img_hdr_$(notdir $(basename $($(PROCESSOR).SRAM_BIN))).cfg

ALL_ADSP_BINS := $(ALL_ADSP_BINS) \
  $($(PROCESSOR).DRAM_BIN) $($(PROCESSOR).IRAM_BIN) $($(PROCESSOR).SRAM_BIN)

ifeq (true,$(BUILD_ADSP_DEFAULT_IMAGES))
ADSP_DEFIMG_INSTALLED_EXTRA := $(ADSP_DEFIMG_INSTALLED_EXTRA) \
  $($(PROCESSOR).ELF_FILE) \
  $($(PROCESSOR).TINYSYS_CONFIG_H)
endif

include $(BUILD_DIR)/env_hifi3.mk

# toolchain is unavailable but BUILD_WITH_XTENSA is on? Trigger error
#ifeq (,$(wildcard $(CC)))
#  $(error $(TINYSYS_ADSP): BUILD_WITH_XTENSA is yes, but toolchain $(CC) is not available)
#endif

# Make sure license is ready
ifeq (,$(XTENSAD_LICENSE_FILE))
ifeq (,$(shell ls $${HOME}/.flexlmrc))
  $(error $(TINYSYS_ADSP): XTENSAD_LICENSE_FILE is undefined and $${HOME}/.flexlmrc is missing. Please export XTENSAD_LICENSE_FILE in the build command)
endif
endif

# Include project-specific files only when available
ifneq ($(wildcard $(PROJECT_DIR)/inc),)
  INCLUDES += $(PROJECT_DIR)/inc
endif

ifneq ($(wildcard $(PROJECT_DIR)/src/project.c),)
  C_FILES += $(wildcard $(PROJECT_DIR)/src/project.c)
endif

C_FILES  := $(patsubst ./%,%,$(C_FILES))
S_FILES  := $(patsubst ./%,%,$(S_FILES))
C_OBJS   := $(sort $(C_FILES:%.c=$(MY_BUILT_DIR)/%.o))
S_OBJS   := $(sort $(S_FILES:%.S=$(MY_BUILT_DIR)/%.o))
OBJS     += $(sort $(C_OBJS) $(S_OBJS))

TCM_C_FILES := $(patsubst ./%,%,$(TCM_C_FILES))
TCM_S_FILES := $(patsubst ./%,%,$(TCM_S_FILES))
TCM_C_OBJS  := $(sort $(TCM_C_FILES:%.c=$(MY_BUILT_DIR)/%.o))
TCM_S_OBJS  := $(sort $(TCM_S_FILES:%.S=$(MY_BUILT_DIR)/%.o))
TCM_OBJS    += $(sort $(TCM_C_OBJS) $(TCM_S_OBJS))

$(OBJS) $(TCM_OBJS): $($(PROCESSOR).TINYSYS_CONFIG_H)

CFLAGS += -include $($(PROCESSOR).TINYSYS_CONFIG_H)

INCLUDES := $(call normalize-includes,$(INCLUDES))

DEPS += $(sort $(filter-out %.d,$(MAKEFILE_LIST)))

###########################################################
## Processor-based build targets
###########################################################
$($(PROCESSOR).DRAM_BIN): PRIVATE_BIN_NH := $($(PROCESSOR).DRAM_BIN_NH)
$($(PROCESSOR).DRAM_BIN): PRIVATE_IMG_HDR_CFG := $($(PROCESSOR).DRAM_HDRCFG)
$($(PROCESSOR).DRAM_BIN): $($(PROCESSOR).DRAM_BIN_NH) $($(PROCESSOR).DRAM_HDRCFG) $(DEPS) | $(MKIMAGE)
	@echo '$(TINYSYS_ADSP): BIN   $@'
	$(hide)$(MKIMAGE) $(PRIVATE_BIN_NH) $(PRIVATE_IMG_HDR_CFG) > $@

$($(PROCESSOR).IRAM_BIN): PRIVATE_BIN_NH := $($(PROCESSOR).IRAM_BIN_NH)
$($(PROCESSOR).IRAM_BIN): PRIVATE_IMG_HDR_CFG := $($(PROCESSOR).IRAM_HDRCFG)
$($(PROCESSOR).IRAM_BIN): $($(PROCESSOR).IRAM_BIN_NH) $($(PROCESSOR).IRAM_HDRCFG) $(DEPS) | $(MKIMAGE)
	@echo '$(TINYSYS_ADSP): BIN   $@'
	$(hide)$(MKIMAGE) $(PRIVATE_BIN_NH) $(PRIVATE_IMG_HDR_CFG) > $@

$($(PROCESSOR).SRAM_BIN): PRIVATE_BIN_NH := $($(PROCESSOR).SRAM_BIN_NH)
$($(PROCESSOR).SRAM_BIN): PRIVATE_IMG_HDR_CFG := $($(PROCESSOR).SRAM_HDRCFG)
$($(PROCESSOR).SRAM_BIN): $($(PROCESSOR).SRAM_BIN_NH) $($(PROCESSOR).SRAM_HDRCFG) $(DEPS) | $(MKIMAGE)
	@echo '$(TINYSYS_ADSP): BIN   $@'
	$(hide)$(MKIMAGE) $(PRIVATE_BIN_NH) $(PRIVATE_IMG_HDR_CFG) > $@

$($(PROCESSOR).IRAM_TEXT): PRIVATE_DUMPELF := $(DUMPELF)
$($(PROCESSOR).IRAM_TEXT): $($(PROCESSOR).ELF_FILE)
	@mkdir -p $(dir $@)
	$(hide)$(PRIVATE_DUMPELF) --big-endian --width 64 --base $(CFG_HIFI3_IRAM_ADDRESS) --size $(CFG_HIFI3_IRAM_SIZE) --full $< > $@

$($(PROCESSOR).DRAM_TEXT): PRIVATE_DUMPELF := $(DUMPELF)
$($(PROCESSOR).DRAM_TEXT): $($(PROCESSOR).ELF_FILE)
	@mkdir -p $(dir $@)
	$(hide)$(PRIVATE_DUMPELF) --big-endian --width 64 --base $(CFG_HIFI3_DRAM_ADDRESS) --size $(CFG_HIFI3_DRAM_SIZE) --full $< > $@

$($(PROCESSOR).SRAM_TEXT): PRIVATE_PROCESSOR := $(PROCESSOR)
$($(PROCESSOR).SRAM_TEXT): PRIVATE_DUMPELF := $(DUMPELF)
$($(PROCESSOR).SRAM_TEXT): PRIVATE_MAP := $($(PROCESSOR).MAP_FILE)
$($(PROCESSOR).SRAM_TEXT): $($(PROCESSOR).ELF_FILE)
	@mkdir -p $(dir $@)
	@SRAM_$(PRIVATE_PROCESSOR)_END=$$(sed -nr 's#[[:space:]]+(0x[0-9a-f]+)[[:space:]]+_data_end = .*#\1#gp' $(PRIVATE_MAP)); \
		SRAM_$(PRIVATE_PROCESSOR)_SIZE=$$(printf '0x%x' $$((($${SRAM_$(PRIVATE_PROCESSOR)_END}-$(CFG_HIFI3_SRAM_ADDRESS) + 0x7) & ~0x7))); \
		echo "$(TINYSYS_ADSP): $(PRIVATE_PROCESSOR) SRAM size: $${SRAM_$(PRIVATE_PROCESSOR)_SIZE}" ; \
		echo "$(PRIVATE_DUMPELF) --big-endian --width 64 --base $(CFG_HIFI3_SRAM_ADDRESS) --size $${SRAM_$(PRIVATE_PROCESSOR)_SIZE} --full $< > $@"; \
		$(PRIVATE_DUMPELF) --big-endian --width 64 --base $(CFG_HIFI3_SRAM_ADDRESS) --size $${SRAM_$(PRIVATE_PROCESSOR)_SIZE} --full $< > $@

$($(PROCESSOR).IRAM_BIN_NH): $($(PROCESSOR).IRAM_TEXT)
	@mkdir -p $(dir $@)
	$(hide)$(HEX2BIN_UTIL) $< $@

$($(PROCESSOR).DRAM_BIN_NH): $($(PROCESSOR).DRAM_TEXT)
	@mkdir -p $(dir $@)
	$(hide)$(HEX2BIN_UTIL) $< $@

$($(PROCESSOR).SRAM_BIN_NH): $($(PROCESSOR).SRAM_TEXT)
	@mkdir -p $(dir $@)
	$(hide)$(HEX2BIN_UTIL) $< $@

$($(PROCESSOR).ELF_FILE): PRIVATE_CC := $(CC)
$($(PROCESSOR).ELF_FILE): PRIVATE_INCLUDES := $(INCLUDES)
$($(PROCESSOR).ELF_FILE): PRIVATE_CFLAGS := $(CFLAGS)
$($(PROCESSOR).ELF_FILE): PRIVATE_LDFLAGS := $(LDFLAGS)
$($(PROCESSOR).ELF_FILE): PRIVATE_LIBFLAGS := $(LIBFLAGS)
$($(PROCESSOR).ELF_FILE): PRIVATE_OBJS := $(TCM_OBJS) $(OBJS)
# FIXME: hard-coded LSP
$($(PROCESSOR).ELF_FILE): PRIVATE_LSP := $(PLATFORM_DIR)/lnk-hifi3/$(XT_TOOLS_VERSION)
$($(PROCESSOR).ELF_FILE): PRIVATE_BUILT_DIR := $(MY_BUILT_DIR)
$($(PROCESSOR).ELF_FILE): PRIVATE_PLATFORM_DIR := $(PLATFORM_DIR)
$($(PROCESSOR).ELF_FILE): PRIVATE_PROJECT_DIR := $(PROJECT_DIR)
$($(PROCESSOR).ELF_FILE): $(TCM_OBJS) $(OBJS) $(DEPS)
	@echo '$(TINYSYS_ADSP): BIN   $@'
	@mkdir -p $(dir $@)
	$(hide)$(PRIVATE_CC) $(PRIVATE_CFLAGS) $(PRIVATE_INCLUDES) -o $@ $(PRIVATE_OBJS) -mlsp=$(PRIVATE_LSP) $(PRIVATE_LDFLAGS) $(PRIVATE_LIBFLAGS)

$($(PROCESSOR).DRAM_HDRCFG) $($(PROCESSOR).IRAM_HDRCFG) $($(PROCESSOR).SRAM_HDRCFG): $(DEPS)
	$(call gen-image-header,$(TINYSYS_ADSP))

###########################################################
## TCM specific
###########################################################
# TCM_CFLAGS must wait for fully-defined CLFAGS
TCM_CFLAGS := $(filter-out -ffunction-sections -fdata-sections,$(CFLAGS)) $(TCM_RENAME_FLAGS)

$(TCM_C_OBJS): PRIVATE_CC := $(CC)
$(TCM_C_OBJS): PRIVATE_CFLAGS := $(TCM_CFLAGS)
$(TCM_C_OBJS): PRIVATE_INCLUDES := $(INCLUDES)
$(TCM_C_OBJS): $(MY_BUILT_DIR)/%.o: %.c
	$(compile-c-or-s-to-o)

$(TCM_S_OBJS): PRIVATE_CC := $(CC)
$(TCM_S_OBJS): PRIVATE_CFLAGS := $(TCM_CFLAGS)
$(TCM_S_OBJS): PRIVATE_INCLUDES := $(INCLUDES)
$(TCM_S_OBJS): $(MY_BUILT_DIR)/%.o: %.S
	$(compile-c-or-s-to-o)

-include $(OBJS:.o=.d)
-include $(TCM_OBJS:.o=.d)

else # BUILD_WITH_XTENSA is not enabled. Use default image
$(info $(TINYSYS_ADSP): BUILD_WITH_XTENSA is not enabled. Use default image.)
ifneq (,$(wildcard ../adsp_3rd_party_lib))
ADSP_DEFAULT_IMAGE := ../adsp_3rd_party_lib/$(PLATFORM)/$(PLATFORM)_default_adsp_3rd_party.img
else
ADSP_DEFAULT_IMAGE := ../adsp_lib/$(PLATFORM)/$(PLATFORM)_default_adsp.img
endif
ALL_ADSP_BINS := $(ALL_ADSP_BINS) $(ADSP_DEFAULT_IMAGE)
endif # BUILD_WITH_XTENSA
endif # CFG_MTK_AUDIODSP_SUPPORT is enabled
