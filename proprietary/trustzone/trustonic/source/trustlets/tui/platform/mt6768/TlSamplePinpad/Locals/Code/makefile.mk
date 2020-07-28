################################################################################
#
# SDK Sample Pinpad Trusted Application
#
################################################################################

TBASE_API_LEVEL := 6

#-------------------------------------------------------------------------------
# Output
#-------------------------------------------------------------------------------
### output name without path or extension
#OUTPUT_NAME := tlPinpad${TBASE_API_LEVEL}
OUTPUT_NAME := tlPinpad

### Mobiconvert parameters
TA_UUID := 0789000000000000000000000000000${TBASE_API_LEVEL}
TRUSTLET_UUID := 0789000000000000000000000000000${TBASE_API_LEVEL}

ifeq ($(BUILD_TLSAMPLEPINPAD_AS_SP),TRUE)
TA_SERVICE_TYPE := SP
TA_KEYFILE := Locals/Build/keySpTl.xml
else
TA_SERVICE_TYPE := SYS
TA_KEYFILE := Locals/Build/pairVendorTltSig.pem
endif


TA_INSTANCES := 1 # min: 1; max: 16
ifeq ($(TOOLCHAIN),GNU)
    TA_OPTS += -Werror
endif
ifeq ($(TOOLCHAIN),ARM)
   TA_OPTS += --diag_error=warning
endif

#-------------------------------------------------------------------------------



#  2,555,904 == 0x00270000 # enough for WSVGA, 1024 x 600
# 14,811,136 == 0x00E20000 # enough for Quad HD, 2560 x 1440
HEAP_SIZE_INIT := 14811136

#-------------------------------------------------------------------------------
# Files and include paths - Add your files here
#-------------------------------------------------------------------------------

### A simple scatter file and set SCATTER_FILE.
### Uncomment this to use a custom scatter file $(OUTPUT_NAME).sct
# SCATTER_FILE := $(TRUSTED_APP_DIR)/$(OUTPUT_NAME).sct

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
TRUSTED_APP_DIR ?= Locals/Code
TLSDK_DIR_SRC ?= $(TLSDK_DIR)
#include $(TLSDK_DIR)/trusted_application.mk
include $(TLSDK_DIR)/trustlet.mk
