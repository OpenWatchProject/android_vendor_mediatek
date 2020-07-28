ifneq ($(TEE_DUMP_MAKEFILE_ONLY), true)

# ************************************************************************************************
# Environment
$(info ******************************************)
$(info Trusted Application Build)
$(info ******************************************)

# MODE : Debug, Release
ifeq ($(MODE),)
$(info - MODE  is  not  set,  default  is : Debug)
MODE ?= Debug
endif

# TOOLCHAIN : ARM, GNU
ifeq ($(TOOLCHAIN),)
$(info - TOOLCHAIN is not set, default is : ARM)
TOOLCHAIN ?= ARM
endif

# GP ENTRY : tlMain, GP Entrypoints
ifeq ($(GP_ENTRYPOINTS),)
$(info - GP_ENTRYPOINTS is not set, default is : N)
GP_ENTRYPOINTS ?= N
endif

# TA_INTERFACE_VERSION : major.minor
ifeq ($(TA_INTERFACE_VERSION),)
$(info - TA_INTERFACE_VERSION  is  not  set,  default  is : 0.0)
TA_INTERFACE_VERSION ?= 0.0
endif


# TBASE_API_LEVEL
ifeq ($(TBASE_API_LEVEL),)
$(info - TBASE_API_LEVEL  is  not  set,  default  is : 2)
TBASE_API_LEVEL ?= 2
endif

# HW_FLOATING_POINT
ifeq ($(HW_FLOATING_POINT),)
$(info - HW_FLOATING_POINT  is  not  set,  default  is : N)
HW_FLOATING_POINT ?= N
endif

# TA_ROLLBACK_PROTECTED
ifeq ($(TA_ROLLBACK_PROTECTED),)
$(info - TA_ROLLBACK_PROTECTED  is  not  set,  default  is : N)
TA_ROLLBACK_PROTECTED ?= N
endif

# TEE_MACH_TYPE
ifeq ($(TEE_MACH_TYPE),)
$(info - TEE_MACH_TYPE  is  not  set,  default  is : MTXXXX)
TEE_MACH_TYPE ?= MTXXXX
endif

$(info ************************************************)
$(info MODE                  : $(MODE))
$(info TOOLCHAIN             : $(TOOLCHAIN))
$(info GP-ENTRIES            : $(GP_ENTRYPOINTS))
$(info TA_INTERFACE_VERSION  : $(TA_INTERFACE_VERSION))
$(info TBASE_API_LEVEL       : $(TBASE_API_LEVEL))
$(info HW_FLOATING_POINT     : $(HW_FLOATING_POINT))
$(info TA_ROLLBACK_PROTECTED : $(TA_ROLLBACK_PROTECTED))
$(info TEE_MACH_TYPE         : $(TEE_MACH_TYPE))
$(info ************************************************)

# Check if variables are correctly set
ifneq ($(MODE),Debug)
   ifneq ($(MODE),Release)
      $(error ERROR : MODE value is not correct : $(MODE))
   endif
endif
ifneq ($(TOOLCHAIN),GNU)
   ifneq ($(TOOLCHAIN),ARM)
      $(error ERROR : TOOLCHAIN value is not correct : $(TOOLCHAIN))
   endif
endif
ifneq ($(GP_ENTRYPOINTS),N)
   ifneq ($(GP_ENTRYPOINTS),Y)
      $(error ERROR : GP_ENTRYPOINTS value is not correct : $(GP))
   endif
endif
ifneq ($(HW_FLOATING_POINT),N)
   ifneq ($(HW_FLOATING_POINT),Y)
      $(error ERROR : HW_FLOATING_POINT value is not correct : $(HW_FLOATING_POINT))
   endif
endif
ifneq ($(TA_ROLLBACK_PROTECTED),N)
   ifneq ($(TA_ROLLBACK_PROTECTED),Y)
      $(error ERROR : TA_ROLLBACK_PROTECTED value is not correct : $(TA_ROLLBACK_PROTECTED))
   endif
endif


# Accept old variable names
TRUSTED_APP_DIR ?= $(TRUSTLET_DIR)

TA_UUID ?= $(TRUSTLET_UUID)
TA_UUID := $(shell echo $(TA_UUID) | tr A-Z a-z)
TA_MEMTYPE ?= $(TRUSTLET_MEMTYPE)
TA_SERVICE_TYPE ?= $(TRUSTLET_SERVICE_TYPE)
TA_KEYFILE ?= $(TRUSTLET_KEYFILE)
TA_FLAGS ?= $(TRUSTLET_FLAGS)

# Default values for mostly irrelevant properties
ifeq ($(TA_MEMTYPE),)
TA_MEMTYPE := 2  # 0: internal memory prefered; 1: internal memory used; 2: external memory used
endif

# Convenience values for SERVICE_TYPE
ifneq ($(strip $(TA_SERVICE_TYPE)),2)
   ifneq ($(strip $(TA_SERVICE_TYPE)),3)
      ifneq ($(strip $(TA_SERVICE_TYPE)),SP)
         ifneq ($(strip $(TA_SERVICE_TYPE)),SYS)
            ifneq ($(strip $(BUILD_TRUSTLET_LIBRARY_ONLY)), yes)
               $(error ERROR : TA_SERVICE_TYPE value is not correct : -$(TA_SERVICE_TYPE)-)
            endif
         endif
      endif
   endif
endif
ifeq ($(strip $(TA_SERVICE_TYPE)),SP)
TA_SERVICE_TYPE := 2
endif
ifeq ($(strip $(TA_SERVICE_TYPE)),SYS)
TA_SERVICE_TYPE := 3
endif

# Default values for mostly irrelevant properties
ifeq ($(TA_FLAGS),)
TA_FLAGS := 0 # 0: no flag; 1: permanent; 2: service has no WSM control interface; 4: debuggable
endif
ifeq ($(TA_DEBUGGABLE),Y)
TA_FLAGS := $$(($(TA_FLAGS)|4))
endif

DOWNGRADE_PROTECTED :=
ifeq ($(TA_ROLLBACK_PROTECTED),Y)
   DOWNGRADE_PROTECTED := --downgrade-protected
endif

# Extended memory layout support is by default for TBASE_API_LEVEL>=5
EXTENDED_LAYOUT = $(shell if [ $(TBASE_API_LEVEL) -ge 5 ] ; then echo Y ; else echo NO ; fi)
TA_HEAP_SIZE_INIT_PARAM :=
TA_HEAP_SIZE_MAX_PARAM :=
ifeq ($(call EXTENDED_LAYOUT),Y)
    TA_FLAGS := $$(($(TA_FLAGS)|8))
    ifneq ($(HEAP_SIZE_INIT),)
        TA_HEAP_SIZE_INIT_PARAM :=  -initheapsize $(HEAP_SIZE_INIT)
        ifeq ($(HEAP_SIZE_MAX),)
            TA_HEAP_SIZE_MAX_PARAM :=  -maxheapsize $(HEAP_SIZE_INIT)
        else
            TA_HEAP_SIZE_MAX_PARAM :=  -maxheapsize $(HEAP_SIZE_MAX)
        endif
    endif
else
    ifeq ($(HW_FLOATING_POINT),Y)
        $(error ERROR : require TBASE_API_LEVEL >= 5 to enable HW_FLOATING_POINT)
    endif
endif

# Default values for mostly irrelevant properties
TA_INSTANCES ?= $(TRUSTLET_INSTANCES)
ifeq ($(TA_INSTANCES),)
TA_INSTANCES := 16 # min: 1; max: 16
endif

TA_ADD_FLAGS ?= $(TRUSTLET_ADD_FLAGS)

TA_MOBICONFIG_USE ?= $(TRUSTLET_MOBICONFIG_USE)
TA_MOBICONFIG_KEY ?= $(TRUSTLET_MOBICONFIG_KEY)
TA_MOBICONFIG_KID ?= $(TRUSTLET_MOBICONFIG_KID)

TA_OPTS ?= $(TRUSTLET_OPTS)

TASDK_DIR ?= $(TLSDK_DIR)
INTERNAL_TASDK_DIR ?= $(INTERNAL_TLSDK_DIR)
TASDK_DIR_SRC ?= $(TLSDK_DIR_SRC)

GPENTRY_LIB :=
GP_LEVEL :=
GP_UUIDKEYFILE :=

GPAPI_LIB := $(INTERNAL_TASDK_DIR)/Bin/GpApi/GpApi.lib
ifeq ($(GP_ENTRYPOINTS),Y)
    GPENTRY_LIB := $(INTERNAL_TASDK_DIR)/Bin/GpEntry/GpEntry.lib
    GP_LEVEL := -gp_level GP
    ifeq ($(TA_SERVICE_TYPE),2)
        ifeq ("$(wildcard $(TA_UUIDKEYFILE))","")
            $(error ERROR : TA_UUIDKEYFILE has to be set correctly for GP-SPTAs. Currently : $(TA_UUIDKEYFILE))
        endif
        GP_UUIDKEYFILE := -uuidkeyfile $(TA_UUIDKEYFILE)
        GP_UUID  := $(shell java -jar $(TASDK_DIR)/Bin/MobiConvert/MobiConvert.jar $(GP_UUIDKEYFILE) --printuuid | tr -d '-')
    else
        GP_UUID  := $(TA_UUID)
    endif
    $(info GP_UUID              : $(GP_UUID))
endif

# Toolchain set up
ifeq ($(TOOLCHAIN),GNU)
   CROSS=arm-none-eabi
   LINKER_SCRIPT=$(TASDK_DIR)/trusted_application.ld

   CC=$(CROSS_GCC_PATH_BIN)/$(CROSS)-gcc
   LINKER=$(CROSS_GCC_PATH_BIN)/$(CROSS)-ld
   READELF=$(CROSS_GCC_PATH_BIN)/$(CROSS)-readelf
   OBJDUMP=$(CROSS_GCC_PATH_BIN)/$(CROSS)-objdump
   #ASM=$(CROSS_GCC_PATH_BIN)/$(CROSS)-as
   #We use gcc as preprocessor for GNU asm
   ASM=$(CROSS_GCC_PATH_BIN)/$(CROSS)-gcc
   ARCH=$(CROSS_GCC_PATH_BIN)/$(CROSS)-ar
else
   LINKER_SCRIPT=$(TASDK_DIR)/trusted_application.sct

   CC=$(ARM_RVCT_PATH_BIN)/armcc
   ASM=$(ARM_RVCT_PATH_BIN)/armasm
   LINKER=$(ARM_RVCT_PATH_BIN)/armlink
   READELF=$(ARM_RVCT_PATH_BIN)/fromelf
   ARCH=$(ARM_RVCT_PATH_BIN)/armar
endif
# setup SSP (Stack Smashing Protection)
ifeq ($(ENABLE_STACK_PROTECTION),true)
    ifeq ($(TOOLCHAIN),GNU)
        # starting from GCC 4.9 a new flag fstack-protector-strong was
        # introduced that we need to use it applies the protection to a wider
        # range of function than "-fstack-protector" and will cover some
        # corner cases
        SSP_TYPE := -fstack-protector
        GNU_VER_4_9 := 40900
        GNU_CUR_VER := $(shell $(CC) -dumpversion | sed -e 's/\./0/g')
        USE_SSP_STRONG := $(shell [ $(GNU_CUR_VER) -ge $(GNU_VER_4_9) ] && echo true )
        ifeq ($(USE_SSP_STRONG),true)
            SSP_TYPE := -fstack-protector-strong
        endif
        TA_OPTS += $(SSP_TYPE)
    else
        TA_OPTS += --protect_stack
    endif
endif

# OUTPUT_ROOT used to clean build, keep it defined
OUTPUT_ROOT := $(TRUSTED_APP_DIR)/../../Out/Bin

ifeq ($(TOOLCHAIN),GNU)
   OUTPUT_PATH := $(OUTPUT_ROOT)/$(MODE)
else
   OUTPUT_PATH := $(OUTPUT_ROOT)/$(MODE)
endif

OUTPUT_OBJ_PATH=$(OUTPUT_PATH)/obj-local

# ************************************************************************************************
# Trusted application being build
TA_AXF := $(OUTPUT_PATH)/$(OUTPUT_NAME).axf
TA_LST2 := $(OUTPUT_PATH)/$(OUTPUT_NAME).lst2
TA_LIB := $(OUTPUT_PATH)/$(OUTPUT_NAME).lib
ifeq ($(GP_ENTRYPOINTS),Y)
TA_BIN := $(OUTPUT_PATH)/$(TA_UUID).tabin
GP_BIN := $(OUTPUT_PATH)/$(GP_UUID).tabin
TA_LOG_FILE_NAME=$(OUTPUT_PATH)/$(OUTPUT_NAME)_$(GP_UUID).tabin.log
else
TA_BIN := $(OUTPUT_PATH)/$(TA_UUID).tlbin
TA_LOG_FILE_NAME=$(OUTPUT_PATH)/$(OUTPUT_NAME)_$(TA_UUID).tlbin.log
endif
UUID_H := $(OUTPUT_PATH)/../Public/$(OUTPUT_NAME)_uuid.h
TA_MOBICONFIG_USE ?= false

ifeq ($(GP_ENTRYPOINTS),Y)
    GP_TA_CONFIG_FILE ?=
ifneq ($(GP_TA_CONFIG_FILE),)
    GP_TA_PROP_PARAM := --gp_prop write $(TA_AXF) $(GP_TA_CONFIG_FILE)
endif
endif

# ************************************************************************************************
# TASDK
TASDK_SRC_C :=
TASDK_SRC_ASM :=
TASDK_DIR_INC := \
    $(INTERNAL_TASDK_DIR)/Public \
    $(INTERNAL_TASDK_DIR)/Public/MobiCore/inc \
    $(INTERNAL_TASDK_DIR)/Public/GPD_TEE_Internal_API
TLAPI_LIB := $(INTERNAL_TASDK_DIR)/Bin/TlApi/TlApi.lib
TLENTRY_LIB := $(INTERNAL_TASDK_DIR)/Bin/TlEntry/TlEntry.lib

# ************************************************************************************************
# ************************************************************************************************
# ************************************************************************************************
# Architecture specifics
# Should not require changes for Trusted Application

# Toolchain options
ARM_OPT_CC := # init as empty
STD_LIBS ?=
ifeq ($(TOOLCHAIN),GNU)
    ARM_ARCH := ARMv7
    ARM_CHIP := ARMV7_A
    ifeq ($(HW_FLOATING_POINT),Y)
        ARM_OPT_CC += -mcpu=generic-armv7-a
        ARM_OPT_CC += -mfpu=neon-vfpv4
        ARM_OPT_CC += -mfloat-abi=softfp
    else
        ARM_OPT_CC += -mcpu=generic-armv7-a
        ARM_OPT_CC += -mfpu=vfp
        ARM_OPT_CC += -mfloat-abi=soft
    endif
    STD_LIBS += -lm -lc -lgcc

    ARM_OPT_CC += -Werror
else
    ARM_ARCH := ARMv7
    ARM_CHIP := ARMV7_A
    ifeq ($(HW_FLOATING_POINT),Y)
        ARM_OPT_CC += --cpu=cortex-a15
        ARM_OPT_CC += --fpu=VFPv4
    else
        ARM_OPT_CC += --cpu=7-A
        ARM_OPT_CC += --fpu=SoftVFP
    endif
    ARM_OPT_LINK := $(ARM_OPT_CC) # copy cpu/fpu settings

    # Special treatment for --diag_error=warnings, downgrade compiler warnings to remarks
    # Suppress Error: C9931W: Your license for Compiler (feature compiler) will expire in 28 days
    ARM_OPT_CC += --diag_remark=9931
    # Suppress Error: C9933W: Waiting for license...
    ARM_OPT_CC += --diag_remark=9933
    # Treat "#223-D: function declared implicitly" as an error
    ARM_OPT_CC += --diag_error=223

    ARM_OPT_CC += --diag_error=warning
    ARM_OPT_CC += --remarks
    ARM_OPT_CC += --brief_diagnostics
endif
ARM_SHAPE := STD
PLATFORM := $(ARM_CHIP)_$(ARM_SHAPE)

ARM_OPT_CC += -DPLAT=$(PLATFORM)
ARM_OPT_CC += -DARM_ARCH=$(ARM_ARCH) -D__$(ARM_ARCH)__
ARM_OPT_CC += -D__$(ARM_CHIP)__
ARM_OPT_CC += -D__$(PLATFORM)__
ARM_OPT_CC += -D$(ARM_CHIP)_SHAPE=$(ARM_SHAPE)
ARM_OPT_CC += $(TA_OPTS)
ARM_OPT_CC += -DTBASE_API_LEVEL=$(TBASE_API_LEVEL)
ARM_OPT_CC += -DTEE_MACH_TYPE_$(shell echo $(TEE_MACH_TYPE) | tr a-z A-Z)

# ASM options
ARM_OPT_ASM := # init as empty
ifeq ($(TOOLCHAIN),GNU)
    ARM_OPT_ASM += -DPLAT=$(ARM_PLAT)
    ARM_OPT_ASM += -DARM_ARCH=$(ARM_ARCH) -D__$(ARM_ARCH)__
    ARM_OPT_ASM += -D__$(ARM_CHIP)__
    ARM_OPT_ASM += -D__$(ARM_PLAT)__
    ARM_OPT_ASM += -D$(ARM_CHIP)_SHAPE=$(ARM_SHAPE)
else
    ARM_OPT_ASM += --cpreproc_opts='--c90,-DPLAT=$(ARM_PLAT),-DARM_ARCH=$(ARM_ARCH),-D__$(ARM_ARCH)__,-D__$(ARM_CHIP)__,-D__$(ARM_PLAT)__,-D$(ARM_CHIP)_SHAPE=$(ARM_SHAPE)'
endif

ifeq ($(HW_FLOATING_POINT),Y)
    ARM_OPT_ASM += $(CPU_OPT_CC_NEON)
else
    ARM_OPT_ASM += $(CPU_OPT_CC_NO_NEON)
endif

ARM_OPT_ASM += -DTEE_MACH_TYPE_$(shell echo $(TEE_MACH_TYPE) | tr a-z A-Z)

# ************************************************************************************************
# Debug options

ifeq ($(MODE),Debug)
   CC_DBG_OPTS := -DDEBUG --debug

   ifeq ($(TOOLCHAIN),GNU)
        ASM_DBG_OPTS :=  -Wa,-g,--keep-locals
   else
        ASM_DBG_OPTS :=  -g \
                         --keep
   endif

   ifeq ($(TOOLCHAIN),GNU)
      LINK_DBG_OPTS :=
   else
      LINK_DBG_OPTS := --debug
   endif

else
   ifeq ($(TOOLCHAIN),GNU)
      #TBUG-400 LINK_DBG_OPTS := --strip-debug
   else
      LINK_DBG_OPTS := --no_comment_section
      # With --no_debug, the axf decoding into lst2 file does not know
      # about thumb or arm mode. It does not change size of tlbin.
      LINK_DBG_OPTS += --no_debug
   endif

endif

# ************************************************************************************************
# ************************************************************************************************
# ************************************************************************************************
# Standard options

ifeq ($(TA_MOBICONFIG_USE),true)
TA_MOBICONFIG_PARAM := --config --in $(TA_AXF) \
            --out $(TA_AXF).conv \
            --key $(TA_MOBICONFIG_KEY) \
            --kid $(TA_MOBICONFIG_KID)
TA_PARAM := -servicetype $(TA_SERVICE_TYPE) \
            -numberofthreads 1 \
            -numberofinstances $(TA_INSTANCES) \
            -memtype $(TA_MEMTYPE) \
            -flags $(TA_FLAGS) \
            -bin $(TA_AXF).conv \
            -output $(TA_BIN) \
            $(GP_LEVEL) \
            $(GP_UUIDKEYFILE) \
            -interfaceversion $(TA_INTERFACE_VERSION) \
            $(DOWNGRADE_PROTECTED) \
            $(TA_HEAP_SIZE_INIT_PARAM) \
            $(TA_HEAP_SIZE_MAX_PARAM) \
            $(TA_ADD_FLAGS)
else
TA_PARAM := -servicetype $(TA_SERVICE_TYPE) \
            -numberofthreads 1 \
            -numberofinstances $(TA_INSTANCES) \
            -memtype $(TA_MEMTYPE) \
            -flags $(TA_FLAGS) \
            -bin $(TA_AXF) \
            -output $(TA_BIN) \
            $(GP_LEVEL) \
            $(GP_UUIDKEYFILE) \
            -interfaceversion $(TA_INTERFACE_VERSION) \
            $(DOWNGRADE_PROTECTED) \
            $(TA_HEAP_SIZE_INIT_PARAM) \
            $(TA_HEAP_SIZE_MAX_PARAM) \
            $(TA_ADD_FLAGS)
endif

ifeq ($(TA_KEYFILE),)
TA_PARAM += --nosign
else
TA_PARAM += -keyfile $(TA_KEYFILE)
endif

TA_CC_OPTS ?=
ifeq ($(TOOLCHAIN),GNU)
CC_OPTS :=  -mthumb \
            -D__THUMB__ \
            -O3 \
            -Os \
            -mthumb-interwork \
            $(TA_CC_OPTS) \
            $(ARM_OPT_CC) \
            $(CC_DBG_OPTS) \
            ${ARMCC_COMPILATION_FLAGS}

ifneq ($(ARMCC_SHORT_ENUMS),YES)
CC_OPTS += -fno-short-enums
endif

else
CC_OPTS :=  -D__ARMCC__ \
            --thumb \
            -D__THUMB__ \
            --apcs=interwork \
            --reduce_paths \
            --diag_style=gnu \
            --depend_format=unix_escaped \
            --no_depend_system_headers \
            --gnu \
            --bss_threshold=0 \
            --interface_enums_are_32_bit \
            --wchar32 \
            -O3 \
            -Ospace \
            $(TA_CC_OPTS) \
            -J$(ARM_RVCT_PATH_INC) \
            $(ARM_OPT_CC) \
            $(CC_DBG_OPTS) \
            ${ARMCC_COMPILATION_FLAGS}

ifneq ($(ARMCC_SHORT_ENUMS),YES)
CC_OPTS += --enum_is_int
endif

endif
CC_OPTS += -DTRUSTLET
CC_OPTS += -DTRUSTEDAPP

TA_LINK_OPTS ?=
ifeq ($(TOOLCHAIN),GNU)
    ASM_OPTS := -mthumb-interwork \
                -D__THUMB__ \
                $(ARM_OPT_ASM) \
                ${ARMCC_COMPILATION_FLAGS} \
                $(ASM_DBG_OPTS) \
                -c

else
    ASM_OPTS := --arm \
                --apcs=interwork \
                --reduce_paths \
                --diag_style=gnu \
                --cpreproc \
                --cpreproc_opts='-D__ARMCC__,-D__THUMB__' \
                $(ARM_OPT_ASM) \
                $(ASM_DBG_OPTS)
    comma:= ,
    empty:=
    space:= $(empty) $(empty)
    tmp = $(subst $(space),$(comma),$(strip $(ARMCC_COMPILATION_FLAGS)))
    ifneq ($(tmp),)
        ASM_OPTS += --cpreproc_opts='$(tmp)'
    endif
endif

ifeq ($(TOOLCHAIN),GNU)
LINK_OPTS := -nostdlib \
            --gc-sections \
            $(TA_LINK_OPTS) \
            -T $(LINKER_SCRIPT) \
            -o $(TA_AXF) \
            $(LINK_DBG_OPTS) \
            -L $(CROSS_GCC_PATH_LIB) \
            -L $(CROSS_GCC_PATH_LGCC) \
            --fatal-warnings \
            --no-wchar-size-warning
else
LINK_OPTS := --entry _tlEntry \
            --reduce_paths \
            --diag_style=gnu \
            --datacompressor=off \
            --verbose \
            --map \
            --callgraph \
            --remove \
            --symbols \
            $(ARM_OPT_LINK) \
            $(TA_LINK_OPTS) \
            --list=$(OUTPUT_PATH)/$(OUTPUT_NAME).lst \
            --libpath=$(ARM_RVCT_PATH_LIB) \
            --scatter=$(LINKER_SCRIPT) \
            -o $(TA_AXF) \
            $(LINK_DBG_OPTS) \
            --diag_error=warning \
            --remarks

# No section matches pattern *(heap)
LINK_OPTS += --diag_suppress=L6314W
# Pattern *(RW) only matches removed unused sections.
LINK_OPTS += --diag_suppress=L6329W
# Your license for Linker (feature armlink5) will expire in XX days
LINK_OPTS += --diag_suppress=L9931W
# Mark as remark: L9933W: Waiting for license...
LINK_OPTS += --diag_remark=L9933W

endif

ifeq ($(TOOLCHAIN),GNU)
ARCHIVER_OPTS := -rcs
DEP_OPTS = -Wno-builtin-macro-redefined -D__FILE__=\"$(notdir $<)\" -MP -MD -MF $(addsuffix .d,$(basename $@))
else
ARCHIVER_OPTS := --create \
             --debug_symbols \
             -v
DEP_OPTS = --depend $(addsuffix .d,$(basename $@))
endif

# ************************************************************************************************
# Actual sets of files to work on
SRC := $(SRC_C) $(SRC_CPP) $(SRC_ASM)
TASDK_SRC := $(TASDK_SRC_C) $(TASDK_SRC_ASM)

SRC_OBJ = $(addprefix $(OUTPUT_OBJ_PATH)/,$(join $(dir $(SRC)), $(addsuffix .o,$(notdir $(basename $(SRC))))))
TASDK_OBJ = $(addprefix $(OUTPUT_OBJ_PATH)/,$(addsuffix .o,$(notdir $(basename $(TASDK_SRC)))))
OBJ := $(SRC_OBJ) $(TASDK_OBJ)

ifeq ($(TOOLCHAIN),GNU)
   INC := $(INCLUDE_DIRS:%=-I%) $(TASDK_DIR_INC:%=-I%) -I$(CROSS_GCC_PATH_INC)
else
   INC := $(INCLUDE_DIRS:%=-I%) $(TASDK_DIR_INC:%=-I%)
endif

LIBS := $(TLENTRY_LIB) $(TLAPI_LIB) $(GPAPI_LIB) ${CUSTOMER_DRIVER_LIBS}
ifeq ($(GP_ENTRYPOINTS),Y)
LIBS += $(GPENTRY_LIB)
endif

LIBS += $(HAL_LIBS)

ifneq ($(strip $(HAL_LIBS)),)
export_includes := $(addsuffix export_includes,$(dir $(HAL_LIBS)))
ifneq ($(wildcard $(export_includes)),)
HAL_INC := $(addprefix -I$(ROOTDIR)/,$(filter-out -I,$(shell cat $(export_includes))))
INC += $(HAL_INC)
endif
endif

ifeq ($(TOOLCHAIN),GNU)
   READ_OPT=-a $(TA_AXF) > $(TA_LST2); $(OBJDUMP) -d $(TA_AXF) >> $(TA_LST2)
   C99=-std=c99
else
   READ_OPT=--text -c -d -e -t -z --datasymbols $(TA_AXF) --output $(TA_LST2)
   C99=--c99
endif

# ************************************************************************************************
# Rules

# Build as Library
ifeq ($(BUILD_TRUSTLET_LIBRARY_ONLY),yes)
ifeq ($(GP_ENTRYPOINTS),Y)
all : $(UUID_H) $(TA_LIB)
else
all : $(TA_LIB)
endif
else # Build as Binary
ifeq ($(GP_ENTRYPOINTS),Y)
all : $(UUID_H) $(GP_BIN)
else
all : $(TA_BIN)
endif
endif

$(TA_LIB) : $(OBJ)
	mkdir -p $(dir $@)
	$(ARCH) $(ARCHIVER_OPTS) $(TA_LIB) $(OBJ)

$(TA_AXF) : $(OBJ) $(LIBS)
	$(info ******************************************)
	$(info ** LINKER ********************************)
	$(info ******************************************)
	mkdir -p $(dir $@)
	$(LINKER) $(LINK_OPTS) $(OBJ) $(if $(strip $(LIBS)),--start-group $(LIBS) --end-group) $(STD_LIBS)

$(TA_BIN) : $(TA_AXF)
	mkdir -p $(dir $@)
ifeq ($(TA_MOBICONFIG_USE),true)
	$(info ******************************************)
	$(info ** MobiConfig ****************************)
	$(info ******************************************)
	java -jar $(COMP_PATH_MobiConfig)/Bin/MobiConfig.jar $(TA_MOBICONFIG_PARAM)
endif
	$(info ******************************************)
	$(info ** READELF & MobiConvert *****************)
	$(info ******************************************)
	$(READELF) $(READ_OPT)
ifneq ($(GP_TA_PROP_PARAM),)
	java -jar $(TASDK_DIR)/Bin/MobiConvert/MobiConvert.jar $(GP_TA_PROP_PARAM)
endif
	java -jar $(TASDK_DIR)/Bin/MobiConvert/MobiConvert.jar $(TA_PARAM) >$(TA_LOG_FILE_NAME)
ifeq ($(TA_KEYFILE),)
	mv $(TA_BIN).raw $(TA_BIN)
endif

ifeq ($(TA_SERVICE_TYPE),2)
ifneq ($(GP_BIN),$(TA_BIN))
$(GP_BIN) : $(TA_BIN)
	$(info ******************************************)
	$(info ** Copy tabin with GP UUID name **********)
	$(info ******************************************)
	mkdir -p $(dir $@)
	cp $(TA_BIN) $(GP_BIN)
	rm -f $(TA_BIN)
endif
endif

$(UUID_H) :
	$(info ******************************************)
	$(info ** UUID header file creation    **********)
	$(info ******************************************)
	mkdir -p $(dir $@)
	mkdir -p $(OUTPUT_PATH)/../Public
	$(TASDK_DIR)/uuid2header.sh $(OUTPUT_NAME) $(GP_UUID) $(UUID_H)

.PHONY: clean

clean :
	rm -rf $(OUTPUT_PATH)/*

clean_all :
	rm -rf $(OUTPUT_ROOT)/*/*

$(OUTPUT_OBJ_PATH)/%.o : $(CURDIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(C99) $(CC_OPTS) $(CC_OPTS2) $(DEP_OPTS) $(INC) -c -o $@ $<

$(OUTPUT_OBJ_PATH)/%.o : $(CURDIR)/%.cpp
	mkdir -p $(dir $@)
	$(CC) -cpp $(CC_OPTS) $(DEP_OPTS) $(INC) -c -o $@ $<

$(OUTPUT_OBJ_PATH)/%.o : $(CURDIR)/%.s
	mkdir -p $(dir $@)
	$(ASM) $(ASM_OPTS) $(DEP_OPTS) $(INC) -o $@ $<

$(OUTPUT_OBJ_PATH)/%.o : $(CURDIR)/%.S
	mkdir -p $(dir $@)
	$(ASM) $(ASM_OPTS) $(DEP_OPTS) $(INC) -o $@ $<

# ************************************************************************************************

ifeq ($(TOOLCHAIN),GNU)
$(TA_AXF): $(LINKER_SCRIPT)
else
$(TA_AXF): $(TASDK_DIR)/trusted_application.sct
endif

ifeq ($(TA_MOBICONFIG_USE),true)
$(TA_BIN): $(COMP_PATH_MobiConfig)/Bin/MobiConfig.jar $(TA_MOBICONFIG_KEY)
endif

$(TA_BIN): $(TASDK_DIR)/Bin/MobiConvert/MobiConvert.jar $(TA_UUIDKEYFILE) $(TA_KEYFILE)
$(UUID_H): $(TASDK_DIR)/Bin/MobiConvert/MobiConvert.jar $(TA_UUIDKEYFILE) $(TASDK_DIR)/uuid2header.sh
$(OBJ) $(UUID_H) $(TA_AXF) $(TA_BIN): $(filter-out %.d,$(MAKEFILE_LIST)) $(TEE_ADDITIONAL_DEPENDENCIES)
-include $(wildcard $(addsuffix .d,$(basename $(OBJ))))
endif #ifneq ($(TEE_DUMP_MAKEFILE_ONLY), true)
