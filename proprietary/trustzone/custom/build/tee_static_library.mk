INTERNAL_LOCAL_CLANG_EXCEPTION_PROJECTS += vendor/mediatek/proprietary/trustzone/common/hal/source
INTERNAL_LOCAL_CLANG_EXCEPTION_PROJECTS += vendor/mediatek/proprietary/trustzone/common/hal/secure
INTERNAL_LOCAL_CLANG_EXCEPTION_PROJECTS += vendor/mediatek/proprietary/trustzone/trustonic

# FIXME start
# DRSDK_DIR_INC
ifeq ($(strip $(TRUSTONIC_TEE_SUPPORT)),yes)


ifeq ($(strip $(TRUSTZONE_PROJECT_MAKEFILE)),)
# For mmm ta/drv build, manually include .mk (copy from common_config.mk)
TRUSTZONE_PROJECT_MAKEFILE := $(wildcard $(addprefix vendor/mediatek/proprietary/trustzone/custom/build/project/,common.mk $(MTK_PLATFORM_DIR).mk $(MTK_BASE_PROJECT).mk $(MTK_TARGET_PROJECT).mk))
include $(TRUSTZONE_PROJECT_MAKEFILE)
$(info MTK_TEE_GP_SUPPORT = $(MTK_TEE_GP_SUPPORT))
$(info MTK_IN_HOUSE_TEE_SUPPORT = $(MTK_IN_HOUSE_TEE_SUPPORT))
$(info TRUSTONIC_TEE_SUPPORT = $(TRUSTONIC_TEE_SUPPORT))
$(info TRUSTONIC_TEE_VERSION = $(TRUSTONIC_TEE_VERSION))
$(info MICROTRUST_TEE_SUPPORT = $(MICROTRUST_TEE_SUPPORT))
$(info MICROTRUST_TEE_LITE_SUPPORT = $(MICROTRUST_TEE_LITE_SUPPORT))
$(info MTK_GOOGLE_TRUSTY_SUPPORT = $(MTK_GOOGLE_TRUSTY_SUPPORT))
$(info MTK_SOTER_SUPPORT = $(MTK_SOTER_SUPPORT))
else
# For mmm trustzone/full build, already includes mk in common_config.mk, do nothing!
endif

TEE_INTERNAL_BSP_PATH := $(MTK_PATH_SOURCE)/trustzone/trustonic/internal/bsp
ifneq ($(wildcard $(TEE_INTERNAL_BSP_PATH)/platform/$(MTK_PLATFORM_DIR)),)
LOCAL_C_INCLUDES += $(TEE_INTERNAL_BSP_PATH)/platform/$(MTK_PLATFORM_DIR)/t-sdk/DrSdk/Out/Public
LOCAL_C_INCLUDES += $(TEE_INTERNAL_BSP_PATH)/platform/$(MTK_PLATFORM_DIR)/t-sdk/DrSdk/Out/Public/MobiCore/inc
LOCAL_C_INCLUDES += $(TEE_INTERNAL_BSP_PATH)/platform/$(MTK_PLATFORM_DIR)/t-sdk/TlSdk/Out/Public
LOCAL_C_INCLUDES += $(TEE_INTERNAL_BSP_PATH)/platform/$(MTK_PLATFORM_DIR)/t-sdk/TlSdk/Out/Public/MobiCore/inc
LOCAL_C_INCLUDES += $(TEE_INTERNAL_BSP_PATH)/platform/$(MTK_PLATFORM_DIR)/t-sdk/TlSdk/Out/Public/GPD_TEE_Internal_API
else ifneq ($(wildcard $(TEE_INTERNAL_BSP_PATH)/common/$(TRUSTONIC_TEE_VERSION)/t-sdk),)
LOCAL_C_INCLUDES += $(TEE_INTERNAL_BSP_PATH)/common/$(TRUSTONIC_TEE_VERSION)/t-sdk/DrSdk/Out/Public
LOCAL_C_INCLUDES += $(TEE_INTERNAL_BSP_PATH)/common/$(TRUSTONIC_TEE_VERSION)/t-sdk/DrSdk/Out/Public/MobiCore/inc
LOCAL_C_INCLUDES += $(TEE_INTERNAL_BSP_PATH)/common/$(TRUSTONIC_TEE_VERSION)/t-sdk/TlSdk/Out/Public
LOCAL_C_INCLUDES += $(TEE_INTERNAL_BSP_PATH)/common/$(TRUSTONIC_TEE_VERSION)/t-sdk/TlSdk/Out/Public/MobiCore/inc
LOCAL_C_INCLUDES += $(TEE_INTERNAL_BSP_PATH)/common/$(TRUSTONIC_TEE_VERSION)/t-sdk/TlSdk/Out/Public/GPD_TEE_Internal_API
endif

# For compatibility
LOCAL_C_INCLUDES += vendor/mediatek/proprietary/trustzone/common/hal/source/tee/common/include

# drv fwk
ifneq ($(TEE_ALL_MODULES.msee_fwk_drv.lib.PATH),)
LOCAL_C_INCLUDES += $(TEE_ALL_MODULES.msee_fwk_drv.lib.PATH)/Locals/Code/public
LOCAL_C_INCLUDES += $(TEE_ALL_MODULES.msee_fwk_ta.lib.PATH)/Locals/Code/public
endif

else ifeq ($(strip $(MICROTRUST_TEE_SUPPORT)),yes)
MICROTRUST_TEE_VERSION ?= 300
# To support mmm way of building ta/drv
ifeq ($(strip $(UT_SDK_DIR)),)
UT_SDK_DIR := vendor/mediatek/proprietary/trustzone/microtrust/ut_sdk/$(MICROTRUST_TEE_VERSION)
endif
# Native CA
LOCAL_C_INCLUDES += $(UT_SDK_DIR)/api/client_api/include
# TA and DRV
LOCAL_C_INCLUDES += $(UT_SDK_DIR)/api/c/include
LOCAL_C_INCLUDES += $(UT_SDK_DIR)/api/pf/gp_native/include
LOCAL_C_INCLUDES += $(UT_SDK_DIR)/api/pf/libuTbta/include
LOCAL_C_INCLUDES += $(UT_SDK_DIR)/api/pf/crypto/include
LOCAL_C_INCLUDES += $(UT_SDK_DIR)/api/pf/ts/include
LOCAL_C_INCLUDES += $(UT_SDK_DIR)/api/pf/rpmb/include
LOCAL_C_INCLUDES += $(UT_SDK_DIR)/api/pf/time/include
LOCAL_C_INCLUDES += $(UT_SDK_DIR)/api/pf/driver_call/include
LOCAL_C_INCLUDES += $(UT_SDK_DIR)/api/pf/driver_framework/include
LOCAL_C_INCLUDES += $(UT_SDK_DIR)/api/sys/base/include

# For compatibility
LOCAL_C_INCLUDES += vendor/mediatek/proprietary/trustzone/common/hal/source/tee/common/include

else ifeq ($(strip $(MICROTRUST_TEE_LITE_SUPPORT)),yes)
MICROTRUST_TEE_VERSION ?= lite
endif

# FIXME end
ifneq ($(TRUSTZONE_GCC_PREFIX),)
    LOCAL_CC := $(TRUSTZONE_GCC_PREFIX)gcc
else
    LOCAL_CC := prebuilts/gcc/linux-x86/arm/gcc-arm-none-eabi-4_8-2014q3/bin/arm-none-eabi-gcc
    LOCAL_CXX := prebuilts/gcc/linux-x86/arm/gcc-arm-none-eabi-4_8-2014q3/bin/arm-none-eabi-g++
    ifeq ($(strip $(TRUSTONIC_TEE_SUPPORT)),yes)
        LOCAL_CFLAGS += -D__TRUSTONIC_TEE__=1
    else ifeq ($(strip $(MICROTRUST_TEE_SUPPORT)),yes)
        #LOCAL_CC := $(UT_SDK_DIR)/tools/arm-2011.03/bin/arm-none-linux-gnueabi-gcc
        #LOCAL_CFLAGS += -Wno-missing-field-initializers ### For arm-2011.03
        LOCAL_CFLAGS += -D__MICROTRUST_TEE__=1
    else ifeq ($(strip $(MICROTRUST_TEE_LITE_SUPPORT)),yes)
        LOCAL_CFLAGS += -D__MICROTRUST_TEE__=1
    else ifeq ($(strip $(MTK_GOOGLE_TRUSTY_SUPPORT)),yes)
        LOCAL_CFLAGS += -D__TRUSTY_TEE__=1
    endif ### ifeq ($(strip $(TRUSTONIC_TEE_SUPPORT)),yes)
endif ### ifneq ($(TRUSTZONE_GCC_PREFIX),)

# FIXME start
ifeq ($(MTK_TRUSTZONE_PLATFORM),)
# ARM_OPT_CC
LOCAL_CFLAGS += -DPLAT=ARMV7_A_STD
CAL_C_INCLUDES += $(UT_SDK_DIR)/
LOCAL_CFLAGS += -DARM_ARCH=ARMv7 -D__ARMv7__
LOCAL_CFLAGS += -D__ARMV7_A__
LOCAL_CFLAGS += -D__ARMV7_A_STD__
LOCAL_CFLAGS += -DARMV7_A_SHAPE=STD
#LOCAL_CFLAGS += -DTEE_MACH_TYPE_$(shell echo $(TEE_MACH_TYPE) | tr a-z A-Z)
# CPU_OPT_CC_NO_NEON
ifeq ($(strip $(TRUSTONIC_TEE_SUPPORT)),yes)
LOCAL_CFLAGS += -mcpu=generic-armv7-a
ifeq ($(TARGET_BUILD_VARIANT),eng)
# Check if it's internal dev
ifneq ($(wildcard $(TOP)/vendor/mediatek/proprietary/trustzone/trustonic/secure/),)
LOCAL_CFLAGS += -DMEMORY_MONITOR=1
endif
else
LOCAL_CFLAGS += -DMEMORY_MONITOR=0
endif
endif
LOCAL_CFLAGS += -mfpu=vfp
LOCAL_CFLAGS += -mfloat-abi=soft
endif
# CC_OPTS_BASE
LOCAL_CFLAGS += -mthumb
LOCAL_CFLAGS += -O3
LOCAL_CFLAGS += -mthumb-interwork
LOCAL_CFLAGS += -D__THUMB__
ifeq ($(TARGET_BUILD_VARIANT),eng)
LOCAL_CFLAGS += -DDEBUG --debug
endif
#LOCAL_CFLAGS += -Werror
#LOCAL_CFLAGS += -Wall
#LOCAL_CFLAGS += -Wextra
# FIXME end

#LOCAL_CLANG := false
# CLANG build need
#LOCAL_CFLAGS += -target arm-none-linux-eabi
#LOCAL_CFLAGS += -m32

# FIXME start
# C99
LOCAL_CONLYFLAGS += -std=c99
# FIXME end

LOCAL_CXX_STL := none

LOCAL_FORCE_STATIC_EXECUTABLE := true

LOCAL_IS_HOST_MODULE :=

# FIXME start
# LINK_OPTS
LOCAL_LDFLAGS += --verbose --gc-sections --fatal-warnings --no-wchar-size-warning
# STD_LIBS
#LOCAL_LDFLAGS += -lm -lc -lgcc
# FIXME end

# .a/.lib
LOCAL_MODULE_SUFFIX := .lib

LOCAL_MULTILIB := 32

LOCAL_NO_CRT := true

LOCAL_NO_DEFAULT_COMPILER_FLAGS := true

LOCAL_NO_LIBGCC := true

LOCAL_SANITIZE := never

LOCAL_SYSTEM_SHARED_LIBRARIES :=
LOCAL_NO_PIC := true
LOCAL_CFLAGS_32 := -fno-PIC

ifeq ($(strip $(MICROTRUST_TEE_SUPPORT)),yes)
LOCAL_CFLAGS += -fms-extensions
endif

ifeq ($(strip $(MICROTRUST_TEE_LITE_SUPPORT)),yes)
LOCAL_CFLAGS += -fms-extensions
endif

#######################static_library.mk###########
my_prefix := TARGET_
include $(BUILD_SYSTEM)/multilib.mk

ifndef my_module_multilib
# libraries default to building for both architecturess
my_module_multilib := both
endif

LOCAL_2ND_ARCH_VAR_PREFIX :=
include $(BUILD_SYSTEM)/module_arch_supported.mk

ifeq ($(my_module_arch_supported),true)

ifeq ($(strip $(LOCAL_MODULE_CLASS)),)
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
endif
ifeq ($(strip $(LOCAL_MODULE_SUFFIX)),)
LOCAL_MODULE_SUFFIX := .a
endif
LOCAL_UNINSTALLABLE_MODULE := true
ifneq ($(strip $(LOCAL_MODULE_STEM)$(LOCAL_BUILT_MODULE_STEM)),)
$(error $(LOCAL_PATH): Cannot set module stem for a library)
endif
ifeq ($(PLATFORM_VERSION),R)
include $(TRUSTZONE_CUSTOM_BUILD_PATH)/hal_binary.mk
else
include $(TRUSTZONE_CUSTOM_BUILD_PATH)/build_binary.mk
endif
$(LOCAL_BUILT_MODULE) : $(built_whole_libraries)
$(LOCAL_BUILT_MODULE) : $(all_objects)
	$(transform-o-to-static-lib)

endif

ifdef TARGET_2ND_ARCH
LOCAL_2ND_ARCH_VAR_PREFIX := $(TARGET_2ND_ARCH_VAR_PREFIX)
include $(BUILD_SYSTEM)/module_arch_supported.mk

ifeq ($(my_module_arch_supported),true)
# Build for TARGET_2ND_ARCH
OVERRIDE_BUILT_MODULE_PATH :=
LOCAL_BUILT_MODULE :=
LOCAL_INSTALLED_MODULE :=
LOCAL_INTERMEDIATE_TARGETS :=


ifeq ($(strip $(LOCAL_MODULE_CLASS)),)
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
endif
ifeq ($(strip $(LOCAL_MODULE_SUFFIX)),)
LOCAL_MODULE_SUFFIX := .a
endif
LOCAL_UNINSTALLABLE_MODULE := true
ifneq ($(strip $(LOCAL_MODULE_STEM)$(LOCAL_BUILT_MODULE_STEM)),)
$(error $(LOCAL_PATH): Cannot set module stem for a library)
endif
ifeq ($(PLATFORM_VERSION),R)
include $(TRUSTZONE_CUSTOM_BUILD_PATH)/hal_binary.mk
else
include $(TRUSTZONE_CUSTOM_BUILD_PATH)/build_binary.mk
endif
$(LOCAL_BUILT_MODULE) : $(built_whole_libraries)
$(LOCAL_BUILT_MODULE) : $(all_objects)
	$(transform-o-to-static-lib)

endif

LOCAL_2ND_ARCH_VAR_PREFIX :=

endif # TARGET_2ND_ARCH

my_module_arch_supported :=


