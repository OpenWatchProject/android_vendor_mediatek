
# The name of the target file, without any path prepended.
# This duplicates logic from base_rules.mk because we need to
# know its results before base_rules.mk is included.
include $(BUILD_SYSTEM)/configure_module_stem.mk

intermediates := $(call local-intermediates-dir,,$(LOCAL_2ND_ARCH_VAR_PREFIX))

# Define the target that is the unmodified output of the linker.
# The basename of this target must be the same as the final output
# binary name, because it's used to set the "soname" in the binary.
# The includer of this file will define a rule to build this target.
linked_module := $(intermediates)/LINKED/$(my_built_module_stem)

ALL_ORIGINAL_DYNAMIC_BINARIES += $(linked_module)

# Because TARGET_SYMBOL_FILTER_FILE depends on ALL_ORIGINAL_DYNAMIC_BINARIES,
# the linked_module rules won't necessarily inherit the PRIVATE_
# variables from LOCAL_BUILT_MODULE.  This tells binary.make to explicitly
# define the PRIVATE_ variables for linked_module as well as for
# LOCAL_BUILT_MODULE.
LOCAL_INTERMEDIATE_TARGETS := $(linked_module)

###################################
include $(GZ_BUILD_SYSTEM)/binary.mk
###################################

###########################################################
## Pack relocation tables
###########################################################
relocation_packer_input := $(linked_module)
relocation_packer_output := $(intermediates)/PACKED/$(my_built_module_stem)

my_pack_module_relocations := false
ifneq ($(DISABLE_RELOCATION_PACKER),true)
    my_pack_module_relocations := $(firstword \
      $(LOCAL_PACK_MODULE_RELOCATIONS_$($(my_prefix)$(LOCAL_2ND_ARCH_VAR_PREFIX)ARCH)) \
      $(LOCAL_PACK_MODULE_RELOCATIONS))
endif

ifeq ($(my_pack_module_relocations),)
  my_pack_module_relocations := $($(LOCAL_2ND_ARCH_VAR_PREFIX)TARGET_PACK_MODULE_RELOCATIONS)
endif

# Do not pack relocations for executables. Because packing results in
# non-zero p_vaddr which causes kernel to load executables to lower
# address (starting at 0x8000) http://b/20665974
ifeq ($(LOCAL_MODULE_CLASS),EXECUTABLES)
  my_pack_module_relocations := false
endif

# TODO (dimitry): Relocation packer is not yet available for darwin
ifneq ($(HOST_OS),linux)
  my_pack_module_relocations := false
endif

ifeq (true,$(my_pack_module_relocations))
# Pack relocations
$(relocation_packer_output): $(relocation_packer_input) | $(ACP)
	$(pack-elf-relocations)
else
$(relocation_packer_output): $(relocation_packer_input) | $(ACP)
	@echo "target Unpacked: $(PRIVATE_MODULE) ($@)"
	$(copy-file-to-target)
endif

###########################################################
## Store a copy with symbols for symbolic debugging
###########################################################
ifeq ($(LOCAL_UNSTRIPPED_PATH),)
my_unstripped_path := $(TARGET_OUT_UNSTRIPPED)/$(patsubst $(PRODUCT_OUT)/%,%,$(my_module_path))
else
my_unstripped_path := $(LOCAL_UNSTRIPPED_PATH)
endif
symbolic_input := $(relocation_packer_output)
symbolic_output := $(my_unstripped_path)/$(my_installed_module_stem)
$(symbolic_output) : $(symbolic_input) | $(ACP)
	@echo "target Symbolic: $(PRIVATE_MODULE) ($@)"
	$(copy-file-to-target)

###########################################################
## Strip
###########################################################
strip_input := $(symbolic_output)
strip_output := $(LOCAL_BUILT_MODULE)

my_strip_module := $(firstword \
  $(LOCAL_STRIP_MODULE_$($(my_prefix)$(LOCAL_2ND_ARCH_VAR_PREFIX)ARCH)) \
  $(LOCAL_STRIP_MODULE))
ifeq ($(my_strip_module),)
  my_strip_module := true
endif

$(strip_output): PRIVATE_STRIP := $($(LOCAL_2ND_ARCH_VAR_PREFIX)TARGET_STRIP)
$(strip_output): PRIVATE_OBJCOPY := $($(LOCAL_2ND_ARCH_VAR_PREFIX)TARGET_OBJCOPY)
$(strip_output): PRIVATE_READELF := $($(LOCAL_2ND_ARCH_VAR_PREFIX)TARGET_READELF)
ifeq ($(my_strip_module),no_debuglink)
$(strip_output): PRIVATE_NO_DEBUGLINK := true
else
$(strip_output): PRIVATE_NO_DEBUGLINK :=
endif

ifneq ($(filter true no_debuglink,$(my_strip_module)),)
# Strip the binary
$(strip_output): $(strip_input) | $($(LOCAL_2ND_ARCH_VAR_PREFIX)TARGET_STRIP)
	$(transform-to-stripped)
else ifeq ($(my_strip_module),keep_symbols)
# Strip only the debug frames, but leave the symbol table.
$(strip_output): $(strip_input) | $($(LOCAL_2ND_ARCH_VAR_PREFIX)TARGET_STRIP)
	$(transform-to-stripped-keep-symbols)

$(LOCAL_INSTALLED_MODULE): PRIVATE_POST_INSTALL_CMD := \
  $($(LOCAL_2ND_ARCH_VAR_PREFIX)TARGET_STRIP) --strip-all $(LOCAL_INSTALLED_MODULE)
endif # my_strip_module
$(strip_output): $(strip_input)
	@echo "target Unstripped: $(PRIVATE_MODULE) ($@)"
	$(copy-file-to-target)

$(cleantarget): PRIVATE_CLEAN_FILES += \
    $(linked_module) \
    $(breakpad_output) \
    $(symbolic_output) \
    $(strip_output)
