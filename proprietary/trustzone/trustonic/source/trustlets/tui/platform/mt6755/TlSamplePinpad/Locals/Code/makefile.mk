################################################################################
#
# <t-sdk Sample Pinpad Trusted Application
#
################################################################################


# Define platform for this driver
# This sets compiler option and selects the DrApi binary.
# Also common MobiCore defines are set.
PLATFORM ?= ARM_V7A_STD

#-------------------------------------------------------------------------------
# Output
#-------------------------------------------------------------------------------
### output name without path or extension
OUTPUT_NAME := tlPinpad

### Mobiconvert parameters
TRUSTLET_UUID := 07890000000000000000000000000000
TRUSTLET_MEMTYPE := 2 # 0: internal memory prefered; 1: internal memory used; 2: external memory used
TRUSTLET_NO_OF_THREADS := 1

ifeq ($(BUILD_TLSAMPLEPINPAD_AS_SP),TRUE)
TRUSTLET_SERVICE_TYPE := 2 # 2: SP trustlet
TRUSTLET_KEYFILE := Locals/Build/keySpTl.xml
else
TRUSTLET_SERVICE_TYPE := 3 # 3: system trustlet
TRUSTLET_KEYFILE := Locals/Build/pairVendorTltSig.pem
endif


TRUSTLET_FLAGS := 0 # 0: no flag; 1: permanent; 2: service has no WSM control interface; 3: both (permanent and service has not WSM control interface)
TRUSTLET_INSTANCES := 2 # min: 1; max: 16
ifeq ($(TOOLCHAIN),GNU)
    TRUSTLET_OPTS += -Werror
endif
ifeq ($(TOOLCHAIN),ARM)
   TRUSTLET_OPTS += --diag_error=warning
endif

#-------------------------------------------------------------------------------

TBASE_API_LEVEL := 3

#-------------------------------------------------------------------------------
# Files and include paths - Add your files here
#-------------------------------------------------------------------------------

### A simple scatter file and set SCATTER_FILE.
### Uncomment this to use a custom scatter file $(OUTPUT_NAME).sct
# SCATTER_FILE := $(TRUSTLET_DIR)/$(OUTPUT_NAME).sct

### Add include path here
INCLUDE_DIRS += \
    Locals/Code/public \
    $(COMP_PATH_DrTui)/Public

### Add source code files for C++ compiler here
SRC_CPP += \
    Locals/Code/tlPinpad.c

### Add source code files for C compiler here
SRC_C += # nothing

### Add source code files for assembler here
SRC_S += # nothing

### Add library files for linker here
CUSTOMER_DRIVER_LIBS += # nothing

### Add feature flags here
ARMCC_COMPILATION_FLAGS += # nothing

#-------------------------------------------------------------------------------
# use generic make file
$(info TLSDK_DIR = $(TLSDK_DIR))
TRUSTLET_DIR ?= Locals/Code
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
include $(TLSDK_DIR)/trustlet.mk

