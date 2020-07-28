# Check to include for source path or prebuilt path
#
# $(1): module's source path to be checked
# $(2): module's prebuilt path to be checked
define mt_module_select
ifneq ($(wildcard $(1)/rules.mk),)
MODULES += $(1)
$$(info source module included: $(1))
else ifneq ($(wildcard $(2)/rules.mk),)
MODULES += $(2)
$$(info prebuilt module included: $(2))
else
$$(error source module not found: $(1))
$$(error prebuilt module not found: $(2))
endif
endef
