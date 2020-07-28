ifneq ($(TEE_DUMP_MAKEFILE_ONLY), true)

# ************************************************************************************************
# Environment
$(info ******************************************)
$(info Driver Build)
$(info ******************************************)

# PLATFORM : ARM_V7A_STD
ifeq ($(PLATFORM),)
$(info - PLATFORM  is  not  set,  default  is : ARM_V7A_STD)
PLATFORM ?= ARM_V7A_STD
endif

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
$(info PLATFORM          : $(PLATFORM))
$(info TOOLCHAIN         : $(TOOLCHAIN))
$(info MODE              : $(MODE))
$(info TBASE_API_LEVEL   : $(TBASE_API_LEVEL))
$(info HW_FLOATING_POINT : $(HW_FLOATING_POINT))
$(info TEE_MACH_TYPE     : $(TEE_MACH_TYPE))
$(info BUILD_DRIVER_LIBRARY_ONLY     : $(BUILD_DRIVER_LIBRARY_ONLY))
$(info ******************************************)

# Check if variables are correctly set
ifneq ($(MODE),Debug)
   ifneq ($(MODE),Release)
       ifneq ($(MODE),$(EXTRA_MODE))
          $(error ERROR : MODE value is not correct : $(MODE))
       endif
   endif
endif
ifneq ($(TOOLCHAIN),GNU)
   ifneq ($(TOOLCHAIN),ARM)
      $(error ERROR : TOOLCHAIN value is not correct : $(TOOLCHAIN))
   endif
endif

# Extended memory layout support is by default for TBASE_API_LEVEL>=5
EXTENDED_LAYOUT = $(shell if [ $(TBASE_API_LEVEL) -ge 5 ] ; then echo Y ; else echo NO ; fi)
DRIVER_HEAP_SIZE_INIT_PARAM :=
DRIVER_HEAP_SIZE_MAX_PARAM :=
ifeq ($(call EXTENDED_LAYOUT),Y)
    DRIVER_FLAGS := $$(($(DRIVER_FLAGS)|8))
    ifneq ($(HEAP_SIZE_INIT),)
        DRIVER_HEAP_SIZE_INIT_PARAM :=  -initheapsize $(HEAP_SIZE_INIT)
        ifeq ($(HEAP_SIZE_MAX),)
            DRIVER_HEAP_SIZE_MAX_PARAM :=  -maxheapsize $(HEAP_SIZE_INIT)
        else
            DRIVER_HEAP_SIZE_MAX_PARAM :=  -maxheapsize $(HEAP_SIZE_MAX)
        endif
    endif
else
    ifeq ($(HW_FLOATING_POINT),Y)
        $(error ERROR : require TBASE_API_LEVEL >= 5 to enable HW_FLOATING_POINT)
    endif
endif

ifneq ($(HW_FLOATING_POINT),N)
   ifneq ($(HW_FLOATING_POINT),Y)
      $(error ERROR : HW_FLOATING_POINT value is not correct : $(HW_FLOATING_POINT))
   endif
endif

ifeq ($(TOOLCHAIN),GNU)
   CROSS=arm-none-eabi
   LINKER_SCRIPT=$(DRSDK_DIR)/driver.ld

   CC=$(CROSS_GCC_PATH_BIN)/$(CROSS)-gcc
   LINKER=$(CROSS_GCC_PATH_BIN)/$(CROSS)-ld
   READELF=$(CROSS_GCC_PATH_BIN)/$(CROSS)-readelf
   OBJDUMP=$(CROSS_GCC_PATH_BIN)/$(CROSS)-objdump
   #ASM=$(CROSS_GCC_PATH_BIN)/$(CROSS)-as
   #We use gcc as preprocessor for GNU asm
   ASM=$(CROSS_GCC_PATH_BIN)/$(CROSS)-gcc
   ARCH=$(CROSS_GCC_PATH_BIN)/$(CROSS)-ar
else
   LINKER_SCRIPT=$(DRSDK_DIR)/dr.sct

   CC=$(ARM_RVCT_PATH_BIN)/armcc
   ASM=$(ARM_RVCT_PATH_BIN)/armasm
   LINKER=$(ARM_RVCT_PATH_BIN)/armlink
   READELF=$(ARM_RVCT_PATH_BIN)/fromelf
   ARCH=$(ARM_RVCT_PATH_BIN)/armar
endif

# OUTPUT_ROOT used to clean build, keep it defined
OUTPUT_ROOT := $(DRIVER_DIR)/../../Out/Bin
ifeq ($(TOOLCHAIN),GNU)
   OUTPUT_PATH := $(OUTPUT_ROOT)/$(MODE)
   OUTPUT_PATH_INDEPENDENT := $(OUTPUT_PATH)
else
   OUTPUT_PATH := $(OUTPUT_ROOT)/$(MODE)
   OUTPUT_PATH_INDEPENDENT := $(OUTPUT_PATH)
endif
OUTPUT_OBJ_PATH=$(OUTPUT_PATH)/obj-local
ifeq ($(BUILD_DRIVER_LIBRARY_ONLY),YES)
OUTPUT_CORE_OBJ_PATH=$(OUTPUT_PATH)/obj-core
endif
OUTPUT_PLAT_OBJ_PATH=$(OUTPUT_PATH)/obj-plat

# ************************************************************************************************
# Driver being built
DR_AXF := $(OUTPUT_PATH)/$(OUTPUT_NAME).axf
DR_BIN := $(OUTPUT_PATH)/$(DRIVER_UUID).drbin
DR_LST2 := $(OUTPUT_PATH)/$(OUTPUT_NAME).lst2
DR_LIB := $(OUTPUT_PATH_INDEPENDENT)/$(OUTPUT_NAME).lib
ifeq ($(BUILD_DRIVER_LIBRARY_ONLY),YES)
DR_CORE_LIB := $(OUTPUT_PATH_INDEPENDENT)/$(OUTPUT_NAME).lib
endif

# ************************************************************************************************
# ************************************************************************************************
# ************************************************************************************************
# Standard options

DR_PARAM := -servicetype $(DRIVER_SERVICE_TYPE) \
            -numberofthreads $(DRIVER_NO_OF_THREADS) \
            -bin $(DR_AXF) \
            -output $(DR_BIN) \
            -d $(DRIVER_ID) \
            -memtype $(DRIVER_MEMTYPE) \
            -flags $(DRIVER_FLAGS) \
            -interfaceversion $(DRIVER_INTERFACE_VERSION) \
            $(DRIVER_HEAP_SIZE_INIT_PARAM) \
            $(DRIVER_HEAP_SIZE_MAX_PARAM)


ifeq ($(DRIVER_KEYFILE),)
DR_PARAM += --nosign
else
DR_PARAM += -keyfile $(DRIVER_KEYFILE)
endif


# ************************************************************************************************
# Actual sets of files to work on
LowerCase = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$(1)))))))))))))))))))))))))))
OBJ := $(firstword $(wildcard $(foreach n,$(MODE) $(call LowerCase,$(MODE)) release debug,$(n)/$(OUTPUT_NAME).axf)) $(MODE)/$(OUTPUT_NAME).axf)
DR_LIB_OBJ := $(wildcard $(basename $(OBJ)).lib)
DR_CORE_LIB_OBJ := $(firstword $(wildcard $(foreach n,$(MODE) $(call LowerCase,$(MODE)) release debug,$(n)/$(OUTPUT_NAME).lib)) $(MODE)/$(OUTPUT_NAME).lib)

ifeq ($(TOOLCHAIN),GNU)
   READ_OPT_BIN=-a $(DR_AXF) > $(DR_LST2); $(OBJDUMP) -d $(DR_AXF) >> $(DR_LST2)
   READ_OPT_LIB=-a $(DR_LIB) > $(DR_LIB).lst2; $(OBJDUMP) -d $(DR_LIB) >> $(DR_LIB).lst2
   C99=-std=c99
else
   READ_OPT_BIN=--text -c -d -e -t -z --datasymbols $(DR_AXF) --output $(DR_LST2)
   READ_OPT_LIB=--text -c -d -e -t -z --datasymbols $(DR_LIB) --output $(DR_LIB).lst2
   C99=--c99
endif

# ************************************************************************************************
# Rules

# Build as Library
ifeq ($(BUILD_DRIVER_LIBRARY_ONLY),YES)
all : $(DR_CORE_LIB)
else # Build as Binary
all : $(DR_BIN)
ifdef DR_LIB_OBJ
all : $(DR_LIB)
endif
endif

$(DR_AXF) : $(OBJ)
	mkdir -p $(dir $@)
	cp -f $< $@

$(DR_BIN) : $(DR_AXF)
	mkdir -p $(dir $@)
ifeq ($(TRUSTLET_MOBICONFIG_USE),true)
	$(info ******************************************)
	$(info ** MobiConfig ****************************)
	$(info ******************************************)
	java -jar $(COMP_PATH_MobiConfig)/Bin/MobiConfig.jar $(DR_MOBICONFIG_PARAM)
endif
	$(info ******************************************)
	$(info ** READELF & MobiConvert *****************)
	$(info ******************************************)
	$(READELF) $(READ_OPT_BIN)
	java -jar $(DRSDK_DIR)/Bin/MobiConvert/MobiConvert.jar $(DR_PARAM) >$(DR_BIN).log
ifeq ($(DRIVER_KEYFILE),)
	mv $(DR_BIN).raw $(DR_BIN)
endif
	cp $(DR_BIN) $(OUTPUT_PATH)/$(DRIVER_UUID).tlbin

ifeq ($(BUILD_DRIVER_LIBRARY_ONLY),YES)
$(DR_CORE_LIB) : $(DR_CORE_LIB_OBJ)
	mkdir -p $(dir $@)
	cp -f $< $@
else
$(DR_LIB) : $(DR_LIB_OBJ)
	mkdir -p $(dir $@)
	cp -f $< $@
endif

# ************************************************************************************************

ifeq ($(TRUSTLET_MOBICONFIG_USE),true)
$(DR_BIN): $(COMP_PATH_MobiConfig)/Bin/MobiConfig.jar
endif
$(DR_BIN): $(DRSDK_DIR)/Bin/MobiConvert/MobiConvert.jar $(DRIVER_KEYFILE)
$(DR_BIN): $(filter-out %.d,$(MAKEFILE_LIST)) $(TEE_ADDITIONAL_DEPENDENCIES)
endif
