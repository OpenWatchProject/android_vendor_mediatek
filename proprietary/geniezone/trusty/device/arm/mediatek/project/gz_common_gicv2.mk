
# Disable VFP and NEON for now
ARM_WITHOUT_VFP_NEON := true

#
# GLOBAL definitions
#

WITH_EL2 := 1

# Print symbols in backtrace function
WITH_SYMBOL_IN_BACKTRACE := 0

# requires linker GC
WITH_LINKER_GC := 1

# Need support for Non-secure memory mapping
WITH_NS_MAPPING := true

# Criterion test framework
WITH_KERNEL_UNITTEST := 0

# Gouda (Genie OS User Space Architecture) unitest
WITH_GOUDA_UNITTEST := 0

# KTEE/MTEE unitest
WITH_MTEE_KERNEL_UNITTEST := 0
WITH_MTEE_USER_UNITTEST := 0

# enable GZ UT or not
GLOBAL_DEFINES += WITH_GZ_UT=0

# select timer
GLOBAL_DEFINES += TIMER_ARM_GENERIC_SELECTED=CNTHP

# do not relocate kernel in physical memory
GLOBAL_DEFINES += WITH_NO_PHYS_RELOCATION=1

# limit heap grows
GLOBAL_DEFINES += HEAP_GROW_SIZE=8192

# limit physical memory to 38 bit to prevert tt_trampiline from getting larger than arm64_kernel_translation_table
GLOBAL_DEFINES += MMU_IDENT_SIZE_SHIFT=38

# do not measure boot time
GLOBAL_DEFINES += WITH_BOOT_TIME_MEASUREMENT=0

# disable kernel unittest by default
GLOBAL_DEFINES += WITH_KERNEL_UNITTEST=$(WITH_KERNEL_UNITTEST)

# make GIC version available to C sources
ifneq ($(WITH_GIC_VERSION),)
GLOBAL_DEFINES += WITH_GIC_VERSION=$(WITH_GIC_VERSION)
endif

# GZ version control
GLOBAL_DEFINES += VERSION_MAJOR=2
GLOBAL_DEFINES += VERSION_MINOR=0
GLOBAL_DEFINES += VERSION_DEV=001

TRUSTY_USER_ARCH := arm

# This project requires trusty IPC
WITH_TRUSTY_IPC := true

