###########################################################
## Generic definitions
###########################################################
# Remove $@ if error occurs
.DELETE_ON_ERROR:

# Turn off suffix build rules built into make
.SUFFIXES:

.PHONY: FORCE
FORCE:

SHELL          := /bin/bash
.DEFAULT_GOAL  := all

TINYSYS_SCP    := tinysys-scp
TINYSYS_ADSP   := tinysys-adsp
TINYSYS_LOADER := tinysys-loader
TINYSYS_DO     := tinysys-dos

###########################################################
## Parameter control
###########################################################
PROJECT := $(strip $(PROJECT))
ifeq ($(PROJECT),)
  $(error $(TINYSYS_SCP): project name is required)
endif

ifeq ($(strip $(O)),)
O := $(TINYSYS_SCP)_out
endif

# Verbosity control
# Add 'V=1' with make or 'showcommands' when building Android
V ?= 1
ifeq ($(V),1)
hide :=
else
hide := @
endif

###########################################################
## Common directory locations and generic variables
###########################################################
SOURCE_DIR         := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
DEFAULT_SCP_REPO   := vendor/mediatek/proprietary/tinysys/freertos/source
BUILT_DIR          := $(O)/freertos/source

DRIVERS_DIR            := $(SOURCE_DIR)/drivers
DRIVERS_COMMON_DIR     := $(DRIVERS_DIR)/common
BUILD_DIR              := $(SOURCE_DIR)/build
TOOLS_DIR              := $(SOURCE_DIR)/tools
RTOS_SRC_DIR           := $(SOURCE_DIR)/kernel/FreeRTOS/Source
BASE_DIR               := $(SOURCE_DIR)/project
DO_SERVICE_DIR         := $(SOURCE_DIR)/middleware/DoService
TINYSYS_SCP_BIN        := $(O)/scp.img
TINYSYS_ADSP_BIN       := $(O)/audio_dsp.img
TINYSYS_SECURE_DIR     := $(SOURCE_DIR)/../secure
THIRDPARTY_LIB_DIR     := $(SOURCE_DIR)/../../adsp/license/prebuilt/HIFI3/3rd_party
THIRDPARTY_CM4_LIB_DIR := $(SOURCE_DIR)/../adsp_3rd_party_lib/common/cm4

MKIMAGE            := $(TOOLS_DIR)/mkimage
OBJSIZE            := $(TOOLS_DIR)/objsize
MCHECK             := $(TOOLS_DIR)/memoryReport.py

###########################################################
## ADSP default image
###########################################################
ADSP_DEFIMG_PROJECT_SYMBOL := -default-project

ifneq ($(subst $(ADSP_DEFIMG_PROJECT_SYMBOL),,$(PROJECT)),$(PROJECT))
BUILD_ADSP_DEFAULT_IMAGES := true
endif

###########################################################
## Generic build flow
###########################################################
# Common functions and utilities
include $(BUILD_DIR)/definitions.mk

# Initialize the environment for each processor
include $(BUILD_DIR)/main.mk

ifeq (1,$(V))
  $(info $(TINYSYS_SCP): PROCESSORS=$(PROCESSORS))
  $(info $(TINYSYS_SCP): PROJECT=$(PROJECT))
  $(info $(TINYSYS_SCP): PLATFORM=$(PLATFORM))
  $(info $(TINYSYS_SCP): O=$(O))
  $(info $(TINYSYS_SCP): SOURCE_DIR=$(SOURCE_DIR))
  $(info $(TINYSYS_SCP): ALL_SCP_BINS=$(ALL_SCP_BINS))
endif

SETTING := $(BASE_DIR)/CM4_A/$(PLATFORM)/platform/Setting.ini

###########################################################
## Build targets
###########################################################
# If target is ADSP default image, change the installed file
ifeq (true,$(BUILD_ADSP_DEFAULT_IMAGES))
ifneq (,$(filter %3rdparty,$(PROJECT)))
ADSP_DEFIMG_INSTALLED_DIR := $(O)/default_adsp_3rd_party_images
TINYSYS_ADSP_BIN := $(ADSP_DEFIMG_INSTALLED_DIR)/$(PLATFORM)/$(PLATFORM)_default_adsp_3rd_party.img
else
ADSP_DEFIMG_INSTALLED_DIR := $(O)/default_adsp_images
TINYSYS_ADSP_BIN := $(ADSP_DEFIMG_INSTALLED_DIR)/$(PLATFORM)/$(PLATFORM)_default_adsp.img
endif
endif

all: $(TINYSYS_SCP_BIN) $(TINYSYS_ADSP_BIN) ;
scp: $(TINYSYS_SCP_BIN) ;
adsp: $(TINYSYS_ADSP_BIN) ;

ifneq (,$(ALL_SCP_BINS))
SORTED_SCP_BINS := $(call sort_tinysys_binaries,$(ALL_SCP_BINS),CM4)

$(TINYSYS_SCP_BIN): PRIVATE_DRAM_BINS := $(ALL_SCP_DRAM_BINS)
$(TINYSYS_SCP_BIN): $(SORTED_SCP_BINS)
	@mkdir -p $(dir $@)
	@echo '$(TINYSYS_SCP): BIN   $@'
	@VALID_DRAMS=''; \
	for i in $(PRIVATE_DRAM_BINS); do \
		[ -s "$${i}" ] && VALID_DRAMS="$${VALID_DRAMS} $${i}"; \
	done; \
	echo "$(TINYSYS_SCP): cache images: $${VALID_DRAMS}"; \
	echo "cat $^ $${VALID_DRAMS} > $@"; \
	cat $^ $${VALID_DRAMS} > $@
else
$(TINYSYS_SCP_BIN): ;
endif

ifneq (,$(ALL_ADSP_BINS))
$(TINYSYS_ADSP_BIN): $(ALL_ADSP_BINS)
	@mkdir -p $(dir $@)
	@echo '$(TINYSYS_ADSP): BIN   $@'
	$(hide)cat $^ > $@
ifeq (true,$(BUILD_ADSP_DEFAULT_IMAGES))
ifneq (,$(ADSP_DEFIMG_INSTALLED_EXTRA))
	$(hide)cp -f $(ADSP_DEFIMG_INSTALLED_EXTRA) $(ADSP_DEFIMG_INSTALLED_DIR)/$(PLATFORM)
endif
endif
else
$(TINYSYS_ADSP_BIN): ;
endif

clean:
	rm -rf $(TINYSYS_SCP_BIN) $(TINYSYS_ADSP_BIN) $(O)
