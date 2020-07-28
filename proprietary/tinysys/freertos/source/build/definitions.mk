QUOTE   := "
SQUOTE  := '
PERCENT := %
COMMA   := ,

CLEAR_FEATURE_VARS := $(BUILD_DIR)/clear_feature_vars.mk
REGISTER_FEATURE   := $(BUILD_DIR)/register_feature.mk
CLEAR_DO_VARS      := $(BUILD_DIR)/clear_do_vars.mk
REGISTER_DO        := $(BUILD_DIR)/register_do.mk

###########################################################
## Look up the root directory of Android code base.
## Return the path if found, otherwise empty
###########################################################

define find_Android_root_dir
$(shell \
	p=$(PWD); \
	( \
	while [ $${p} != '/' ]; do \
		[ -d '$(DEFAULT_SCP_REPO)' ] && echo $${p} && break; \
		cd ..; \
		p=$${PWD}; \
	done \
	) \
)
endef

###########################################################
## Get processor and platform names by project path.
## Return $(PROCESSOR):$(PLATFORM) string.
## Project path format: $(PROCESSOR)/$(PLATFORM)/$(PROJECT)
###########################################################

# $(1): path of project directory
define get_processor_and_platform
$(eval my_path_words := $(subst /, ,$(1))) \
$(if $(filter 3,$(words $(my_path_words))), \
  $(word 1,$(my_path_words)):$(word 2,$(my_path_words)), \
  $(error Project path layout ($(1)) is incorrect) \
)
endef

###########################################################
## Get processor(s) by platform path.
## Return $(PROCESSOR) string.
## Platform path format: $(PROCESSOR)/$(PLATFORM)
###########################################################

# $(1): path of platform directory
define get_processors_from_platform_paths
$(foreach p,$(1),\
  $(eval my_path_words := $(subst /, ,$(p))) \
  $(word 1,$(my_path_words)) \
) \
$(eval my_path_words :=)
endef

###########################################################
## Create the variable $(PROCESSOR).CONFIG_OPTIONS that
## contains $(CFG_NAME)=$($(CFG_NAME)) pairs from
## given configuration file list for each processor.
##
## The purpose is to collect all config options from given
## config files, which belong to different processors, and
## stash their final values.
###########################################################

# $1: List of configuration files that registers config options
define stash_config_options
$(eval _vars :=) \
$(foreach f,$(strip $(1)), \
  $(eval _vars += $(shell sed -nr 's#^[[:space:]]*(CFG_[0-9A-Z_]+)[ \t]*[:+?]?=.*#\1#gp' $(f))) \
) \
$(foreach v,$(_vars), \
  $(if $($(v)),,$(eval $(v) := no)) \
  $(eval $(v) := $(subst $(QUOTE),\$(QUOTE),$($(v)))) \
  $(eval $(v) := $(subst $(SQUOTE),\$(SQUOTE),$($(v)))) \
  $(eval $(PROCESSOR).CONFIG_OPTIONS += $(strip $(v))=$(strip $($(v)))) \
) \
$(eval $(PROCESSOR).CONFIG_OPTIONS := $(sort $($(PROCESSOR).CONFIG_OPTIONS))) \
$(eval _vars :=)
endef

###########################################################
## Sort required Tiny System intermediate binaries and
## print the result.
## For each processor, loader must precede tinysys binary.
###########################################################

# $(1): the list of unordered binary file paths
# $(2): prefix of processors to be filtered
define sort_tinysys_binaries
$(eval _arg := $(strip $(1))) \
$(if $(_arg),, \
  $(error $(TINYSYS_SCP): sort_tinysys_binaries: argument missing)) \
$(eval SORTED_TINYSYS_DEPS :=) \
$(foreach p,$(filter $(2)_%,$(PROCESSORS)), \
  $(eval m := $(filter %/$(TINYSYS_LOADER)-$(p).bin,$(_arg))) \
  $(if $(m), \
    $(eval SORTED_TINYSYS_DEPS := $(SORTED_TINYSYS_DEPS) $(m)), \
    $(error $(TINYSYS_SCP): Missing loader image for processor $(p)) \
  ) \
  $(eval m := $(filter %/$(TINYSYS_SCP)-$(p).bin,$(_arg))) \
  $(if $(m), \
    $(eval SORTED_TINYSYS_DEPS := $(SORTED_TINYSYS_DEPS) $(m)), \
    $(error $(TINYSYS_SCP): Missing tinysys image for processor $(p)) \
  ) \
  $(eval m := $(filter %/$(TINYSYS_DO)-$(p).do,$(_arg))) \
  $(if $(m), \
    $(eval SORTED_TINYSYS_DEPS := $(SORTED_TINYSYS_DEPS) $(m)), \
  ) \
) \
$(strip $(SORTED_TINYSYS_DEPS)) \
$(eval m :=) \
$(eval _arg :=)
endef

###########################################################
## Template for compiling C and S files to objects
###########################################################

define compile-c-or-s-to-o
	@echo '$(TINYSYS_SCP): CC      $<'
	@mkdir -p $(dir $@)
	$(hide)$(PRIVATE_CC) $(PRIVATE_CFLAGS) $(PRIVATE_LIBFLAGS) \
		$(PRIVATE_INCLUDES) -MD -MP -MF $(patsubst %.o,%.d,$@) -c $< -o $@
endef

###########################################################
## Template for compiling C and S files to objects without
## .d files
###########################################################

define compile-c-or-s-to-o-without-d
	@echo '$(TINYSYS_SCP): CC      $<'
	@mkdir -p $(dir $@)
	$(hide)$(PRIVATE_CC) $(PRIVATE_CFLAGS) $(PRIVATE_LIBFLAGS) \
		$(PRIVATE_INCLUDES) -c $< -o $@
endef

###########################################################
## Template for creating configuration header
###########################################################

# $1: Wrapper macro name, e.g. __TINYSYS_CONFIG_H
# $2: Config options in KEY=VALUE format, separated with spaces
# $3: Identification string to be displayed in any output
define gen-tinysys-header
	@echo '$(strip $(3)): GEN     $@'
	@mkdir -p $(dir $@)
	$(hide)rm -f $(@).tmp
	@echo '/*' > $(@).tmp; \
	echo ' * Automatically generated file; DO NOT EDIT.' >> $(@).tmp; \
	echo ' */' >> $(@).tmp; \
	echo '#ifndef $(strip $(1))' >> $(@).tmp; \
	echo -e '#define $(strip $(1))\n' >> $(@).tmp; \
	for i in $(2); do \
		KEY="$${i//=*}"; \
		VAL="$${i##*=}"; \
		if [ "$${VAL}" = 'yes' ]; then \
			echo "#define $${KEY}" >> $(@).tmp; \
		elif [ "$${VAL}" = 'no' ]; then \
			echo "/* $${KEY} is not set */" >> $(@).tmp; \
		else \
			echo "#define $${KEY} $${VAL}" >> $(@).tmp; \
		fi; \
	done; \
	echo -e '\n#endif /* $(strip $(1)) */' >> $(@).tmp; \
	if [ -f '$@' ]; then \
		if cmp -s '$@' '$(@).tmp'; then \
			rm '$(@).tmp'; \
			echo '$(3): $(@) is update to date.'; \
		else \
			mv '$(@).tmp' '$@'; \
			echo '$(strip $(3)): Updated $@'; \
		fi; \
	else \
		mv '$(@).tmp' '$@'; \
	fi
endef

###########################################################
## Template for creating image header
###########################################################

# $1: Identification string to be displayed in any output
define gen-image-header
	@echo '$(1): GEN     $@'
	@mkdir -p $(dir $@)
	$(hide)echo 'NAME = $(patsubst img_hdr_%.cfg,%,$(notdir $@))' > $@
endef

###########################################################
## Add prefix -I to given include directories that lack it
###########################################################

# $1: Config option name
define normalize-includes
$(patsubst %,-I%,$(patsubst -I%,%,$(1)))
endef

###########################################################
## Deduce the feature name from config option name
###########################################################

# $1: Config option name
define get-feature-name
$(eval fn := FEATURE_$(patsubst CFG_%_SUPPORT,%,strip $(1))) \
$(if $(fn),$(fn),$(error $(TINYSYS_SCP): Invalid feature config: $(1))) \
$(eval fn :=)
endef

###########################################################
## Deduce the DO name from config option name
###########################################################

# $1: Config option name
define get-do-name
$(eval dn := $(patsubst CFG_%,%,strip $(1))) \
$(if $(dn),$(dn),$(error $(TINYSYS_SCP): Invalid DO config: $(1))) \
$(eval dn :=)
endef

define get_do_undefined_symbols
$(eval _und_symbols :=) \
$(foreach d,$($(1).ALL_DOS), \
  $(eval _do_elf := $($(1).$(d).ELF)) \
  $(eval _und_symbols += $(shell $(READELF) -s --wide $(_do_elf) | grep UND | awk 'NF == 8 && $$7 == "UND" { printf "KEEP(*(.text.%s))\n", $$8 }')) \
) \
$(info Undefined symbols: $(_und_symbols))
$(sort $(_und_symbols))
endef

