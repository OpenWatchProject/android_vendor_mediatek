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

# TEE_MACH_TYPE
ifeq ($(TEE_MACH_TYPE),)
$(info - TEE_MACH_TYPE  is  not  set,  default  is : MTXXXX)
TEE_MACH_TYPE ?= MTXXXX
endif

$(info ******************************************)
$(info MODE                : $(MODE))
$(info TOOLCHAIN           : $(TOOLCHAIN))
$(info GP-ENTRIES          : $(GP_ENTRYPOINTS))
$(info TA_INTERFACE_VERSION: $(TA_INTERFACE_VERSION))
$(info TBASE_API_LEVEL     : $(TBASE_API_LEVEL))
$(info HW_FLOATING_POINT   : $(HW_FLOATING_POINT))
$(info TEE_MACH_TYPE       : $(TEE_MACH_TYPE))
$(info ******************************************)

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
TASDK_DIR_SRC ?= $(TLSDK_DIR_SRC)

GP_LEVEL :=
GP_UUIDKEYFILE :=

ifeq ($(GP_ENTRYPOINTS),Y)
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
   ARCH=$(CROSS_GCC_PATH_BIN)/$(CROSS)-ar
else
   LINKER_SCRIPT=$(TASDK_DIR)/trusted_application.sct

   CC=$(ARM_RVCT_PATH_BIN)/armcc
   ASM=$(ARM_RVCT_PATH_BIN)/armasm
   LINKER=$(ARM_RVCT_PATH_BIN)/armlink
   READELF=$(ARM_RVCT_PATH_BIN)/fromelf
   ARCH=$(ARM_RVCT_PATH_BIN)/armar
endif

# OUTPUT_ROOT used to clean build, keep it defined
#TRUSTED_APP_DIR=Locals/Code
OUTPUT_ROOT := $(TRUSTED_APP_DIR)/../../Out/Bin
ifeq ($(TOOLCHAIN),GNU)
   OUTPUT_PATH := $(OUTPUT_ROOT)/$(MODE)
else
   OUTPUT_PATH := $(OUTPUT_ROOT)/$(MODE)
endif

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
TA_MOBICONFIG_USE ?= false


# ************************************************************************************************
# ************************************************************************************************
# ************************************************************************************************

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
            $(TA_HEAP_SIZE_INIT_PARAM) \
            $(TA_HEAP_SIZE_MAX_PARAM) \
            $(TA_ADD_FLAGS)
endif

ifeq ($(TA_KEYFILE),)
TA_PARAM += --nosign
else
TA_PARAM += -keyfile $(TA_KEYFILE)
endif

# ************************************************************************************************
# Actual sets of files to work on
LowerCase = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$(1)))))))))))))))))))))))))))
ifeq ($(BUILD_TRUSTLET_LIBRARY_ONLY),yes)
TA_LIB_OBJ := $(firstword $(wildcard $(foreach n,$(MODE) $(call LowerCase,$(MODE)) release debug,$(n)/$(OUTPUT_NAME).lib)) $(MODE)/$(OUTPUT_NAME).lib)
else
OBJ := $(firstword $(wildcard $(foreach n,$(MODE) $(call LowerCase,$(MODE)) release debug,$(n)/$(OUTPUT_NAME).axf)) $(MODE)/$(OUTPUT_NAME).axf)
endif

ifeq ($(TOOLCHAIN),GNU)
   READ_OPT=-a $(TA_AXF) > $(TA_LST2)
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
all : $(TA_LIB)
else
all : $(TA_LIB)
endif
else # Build as Binary
ifeq ($(GP_ENTRYPOINTS),Y)
all : $(GP_BIN)
else
all : $(TA_BIN)
endif
endif

$(TA_LIB) : $(TA_LIB_OBJ)
	mkdir -p $(dir $@)
	cp -f $< $@

$(TA_AXF) : $(OBJ)
	mkdir -p $(dir $@)
	cp -f $< $@

$(TA_BIN) : $(TA_AXF)
	mkdir -p $(dir $@)
ifeq ($(TA_MOBICONFIG_USE),true)
	$(info ******************************************)
	$(info ** MobiConfig ****************************)
	$(info ******************************************)
	java -jar $(COMP_PATH_MobiConfig)/Bin/MobiConfig.jar $(TA_MOBICONFIG_PARAM)
endif
ifeq ($(TA_CRYPTO_KEY_REPLACEMENT),true)
	$(info ******************************************)
	$(info ** Key Replace ***************************)
	$(info ******************************************)
	python $(TASDK_DIR)/Bin/KeyReplace/replacekeys.py -t $(TA_AXF) -k $(TASDK_DIR)/Bin/KeyReplace/test_key.cfg  -d
endif
	$(info ******************************************)
	$(info ** READELF & MobiConvert *****************)
	$(info ******************************************)
	$(READELF) $(READ_OPT)
	java -jar $(TASDK_DIR)/Bin/MobiConvert/MobiConvert.jar $(TA_PARAM) >$(TA_LOG_FILE_NAME)


# ************************************************************************************************

ifeq ($(TA_MOBICONFIG_USE),true)
$(TA_BIN): $(COMP_PATH_MobiConfig)/Bin/MobiConfig.jar $(TA_MOBICONFIG_KEY)
endif
$(TA_BIN): $(TASDK_DIR)/Bin/MobiConvert/MobiConvert.jar $(TA_UUIDKEYFILE) $(TA_KEYFILE)
$(TA_BIN): $(filter-out %.d,$(MAKEFILE_LIST)) $(TEE_ADDITIONAL_DEPENDENCIES)
endif
