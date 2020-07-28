LOCAL_PATH := $(call my-dir)

# a workaround to replace obsolete ANDROID_BUILD_TOP
AOSP_BUILD_TOP := $(PWD)

include $(CLEAR_VARS)

ifeq ($(strip $(MTK_ENABLE_GENIEZONE)),yes)

GZ_IMAGE := gz
GZ_TARGET_FILE := $(GZ_IMAGE).bin
GZ_TARGET_FILE_IMG := $(GZ_IMAGE).img
GZ_IMAGE_CLEAN := clean-$(GZ_IMAGE)
GZ_IMAGE_MODULE_CLASS := GZ_OBJ
GZ_IMAGE_BUILT_INTERMEDIATES := \
  $(call intermediates-dir-for, \
    $(GZ_IMAGE_MODULE_CLASS),$(GZ_IMAGE))
GZ_INSTALLED_TARGET := $(PRODUCT_OUT)/$(GZ_TARGET_FILE)
GZ_INSTALLED_TARGET_IMG := $(PRODUCT_OUT)/$(GZ_TARGET_FILE_IMG)
GZ_SIGN_IMAGE_TARGET := $(addsuffix -verified$(suffix $(GZ_INSTALLED_TARGET_IMG)),$(basename $(GZ_INSTALLED_TARGET_IMG)))

VM_PREBUILTS_EXTERNAL_PATH = $(LOCAL_PATH)/prebuilts/libs/vm
VM_PREBUILTS_INTERNAL_PATH = $(LOCAL_PATH)/prebuilts/third-party/vm

# Supported plafrom mappings
ifeq ($(strip $(TARGET_BOARD_PLATFORM)),mt6771)
PROJECT := mtk_armv8_el2
GZ_TARGET_BOARD_PLATFORM := mtk_armv8
GZ_ENC := 1
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),mt6765)
PROJECT := mtk_armv8_el2
GZ_TARGET_BOARD_PLATFORM := mtk_armv8
GZ_ENC := 1
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),mt6779)
PROJECT := mtk_armv8_el2
GZ_TARGET_BOARD_PLATFORM := mtk_armv8
GZ_ENC := 1
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),mt6768)
PROJECT := mtk_armv8_el2
GZ_TARGET_BOARD_PLATFORM := mtk_armv8
GZ_ENC := 1
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),mt6785)
PROJECT := mtk_armv8_el2
GZ_TARGET_BOARD_PLATFORM := mtk_armv8
GZ_ENC := 1
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),mt6885)
PROJECT := mtk_armv8_el2
GZ_TARGET_BOARD_PLATFORM := mtk_armv8
GZ_ENC := 1
endif

# co-platform mt3967 with mt6779
ifeq ($(strip $(TARGET_BOARD_PLATFORM)),mt3967)
PROJECT := mtk_armv8_el2
GZ_TARGET_BOARD_PLATFORM := mtk_armv8
GZ_ENC := 0
endif

ifeq ($(strip $(MTK_NEBULA_VM_SUPPORT)),yes)
$(shell cp $(VM_PREBUILTS_INTERNAL_PATH)/nebula/nebula_hee.bin  $(VM_PREBUILTS_EXTERNAL_PATH)/nebula/nebula_hee.bin)
ifneq ($(wildcard $(VM_PREBUILTS_EXTERNAL_PATH)/nebula/nebula_hee.bin),)
PROJECT := mtk_armv8_nebula
else
MTK_NEBULA_VM_SUPPORT=no
endif
endif

ifeq ($(strip $(PROJECT)),)
$(error Unsupported plaform : $(TARGET_BOARD_PLATFORM))
endif

GZ_ROOT_DIR := $(LOCAL_PATH)
GZ_BUILD_SYSTEM := $(LOCAL_PATH)/trusty/device/arm/mediatek/build/
GZ_KEY_PATH := $(GZ_ROOT_DIR)/trusty/device/arm/mediatek/project/iv/$(GZ_TARGET_BOARD_PLATFORM)/
TOOLCHAIN_PATH := $(AOSP_BUILD_TOP)/prebuilts/gcc
MTKTOOLAS_PATH := $(GZ_ROOT_DIR)/scripts

BUILDROOT := $(GZ_IMAGE_BUILT_INTERMEDIATES)
GZ_APP_OUT := $(BUILDROOT)/app

# ------ prebuilt setting ------

PREBUILTS_PATH := $(GZ_ROOT_DIR)/prebuilts/libs
PREBUILTS_LIB_PATH := $(PREBUILTS_PATH)/$(PROJECT)/lib
PREBUILTS_KERNEL_PATH := $(PREBUILTS_PATH)/$(PROJECT)/kernel
PREBUILTS_APP_PATH := $(PREBUILTS_PATH)/$(PROJECT)/app
PREBUILTS_LDSCRIPTS := $(PREBUILTS_PATH)/$(PROJECT)/ldscripts/


# ------ Toolchain setting ------

MKIMAGE := $(PREBUILTS_PATH)/tool/mkimage
TOOLCHAIN_PREFIX := $(TOOLCHAIN_PATH)/linux-x86/aarch64/aarch64-linux-android-4.9/bin/aarch64-linux-android-
TOOLCHAIN_PATH_arm := $(TOOLCHAIN_PATH)/linux-x86/arm/arm-eabi-4.9
TOOLCHAIN_LD_LIBRARY_PATH_arm := $(TOOLCHAIN_PATH_arm)/libexec/gcc/arm-eabi/4.9.4/libmpfr
TOOLCHAIN_PREFIX_arm := $(TOOLCHAIN_PATH_arm)/bin/arm-eabi-

GZ_CC_arm := $(TOOLCHAIN_PREFIX_arm)gcc
GZ_LD_arm := $(TOOLCHAIN_PREFIX_arm)ld
GZ_LD_LIBRARY_PATH_arm := LD_LIBRARY_PATH=$(TOOLCHAIN_LD_LIBRARY_PATH_arm)
LIBGCC_arm := $(shell $(TOOLCHAIN_PREFIX_arm)gcc -print-libgcc-file-name)

GZ_CC := $(TOOLCHAIN_PREFIX)gcc
GZ_LD := $(TOOLCHAIN_PREFIX)ld
GZ_OBJDUMP := $(TOOLCHAIN_PREFIX)objdump
GZ_OBJCOPY := $(TOOLCHAIN_PREFIX)objcopy
GZ_SIZE := $(TOOLCHAIN_PREFIX)size
GZ_NM := $(TOOLCHAIN_PREFIX)nm
GZ_AR := $(TOOLCHAIN_PREFIX)ar
GZ_STRIP := $(TOOLCHAIN_PREFIX)strip
LIBGCC := $(shell $(TOOLCHAIN_PREFIX)gcc -print-libgcc-file-name)

LK_LDSCRIPTS := $(PREBUILTS_LDSCRIPTS)/system-onesegment.ld

GLOBAL_GZ_INCLUDES := \
   $(GZ_ROOT_DIR)/trusty/lib/interface/uuid/include/ \
   $(GZ_ROOT_DIR)/trusty/lib/interface/mtee/include/common/include/ \
   $(GZ_ROOT_DIR)/trusty/lib/lib/mtee_serv/ \
   $(GZ_ROOT_DIR)/trusty/lib/lib/mtee_api/include/ \
   $(GZ_ROOT_DIR)/trusty/lib/lib/libcxx-mtee/include/ \
   $(GZ_ROOT_DIR)/trusty/lib/include/ \
   $(GZ_ROOT_DIR)/trusty/lib/lib/storage/include/ \
   $(GZ_ROOT_DIR)/trusty/lib/interface/storage/include/ \
   $(GZ_ROOT_DIR)/trusty/lib/lib/hwkey/include/ \
   $(GZ_ROOT_DIR)/trusty/lib/interface/hwkey/include/ \
   $(AOSP_BUILD_TOP)/external/boringssl/include/ \
   $(GZ_ROOT_DIR)/trusty/lib/lk/include/ \
   $(GZ_ROOT_DIR)/trusty/lib/lk/arch/arm/include/ \
   $(GZ_ROOT_DIR)/trusty/lib/lk/arch/arm/arm/include

GLOBAL_GZ_INCLUDES += $(GZ_ROOT_DIR)/trusty/lib/lib/libm-mtee/include
GLOBAL_GZ_INCLUDES += $(GZ_ROOT_DIR)/trusty/lib/lib/glibc

GLOBAL_GZ_COMPILEFLAGS := -g -fno-builtin -finline
GLOBAL_GZ_COMPILEFLAGS += -W -Wall -Wno-multichar -Wno-unused-parameter -Wno-unused-function -Wno-unused-label
GLOBAL_GZ_OPTFLAGS := -O2
GLOBAL_GZ_CFLAGS := --std=gnu99 -Werror-implicit-function-declaration -Wstrict-prototypes -Wwrite-strings
GLOBAL_GZ_CFLAGS += -mabi=aapcs-linux -mcpu=cortex-a15 -D__TRUSTZONE_TEE__ -DARM_ISA_ARMV7 -DARM_CPU_ARM7
GLOBAL_GZ_ASMFLAGS := -DASSEMBLY
GLOBAL_GZ_LDFLAGS :=

# For co-exist RPMB parition with GP
ifeq ($(strip $(MTK_TEE_GP_SUPPORT)), yes)
GLOBAL_GZ_CFLAGS += -DMTK_TEE_GP_SUPPORT
endif # MTK_TEE_GP_SUPPORT

# ------ User Task ------

include $(GZ_BUILD_SYSTEM)/prebuilt.mk

GZ_EXECUTABLE := $(GZ_BUILD_SYSTEM)/executable.mk

include $(LOCAL_PATH)/trusty/device/arm/mediatek/app/$(GZ_TARGET_BOARD_PLATFORM).mk

# ------ Build gz.bin  ------

#ifneq ($(strip $(MTK_NEBULA_VM_SUPPORT)),yes)
USER_TASKS_BIN := $(BUILDROOT)/user-tasks.bin
USER_TASKS_OBJ := $(BUILDROOT)/user-tasks.o

KERNEL_PREBUILT_ARCHIVE := $(AOSP_BUILD_TOP)/$(PREBUILTS_KERNEL_PATH)/$(PROJECT)-kernel.a

# Put 5 objects for each line
KERNEL_OBJS_LIST := $(GZ_TARGET_BOARD_PLATFORM).mod.o arm64.mod.o top.mod.o app.mod.o dev.mod.o \
                    arm_generic.mod.o mtk_systimer.mod.o kernel.mod.o memlog.mod.o sm.mod.o trusty.mod.o \
                    platform.mod.o libvmm.mod.o mtcrypto.mod.o sys.mod.o mtsmcall.mod.o \
                    target.mod.o vm.mod.o debug.mod.o fixed_point.mod.o heap.mod.o \
                    libc.mod.a syscall.mod.o uthread.mod.o version.mod.o libfdt.mod.o

#ifeq ($(strip $(MTK_NEBULA_VM_SUPPORT)),yes)
KERNEL_OBJS_LIST += mincrypt.mod.o

KERNEL_PREBUILT_OBJS := $(addprefix $(BUILDROOT)/kernel/,$(KERNEL_OBJS_LIST))
KERNEL_PREBUILT_OBJ := $(firstword $(KERNEL_PREBUILT_OBJS))

ALLUSER_TASK_OBJS := $(wildcard $(PREBUILTS_APP_PATH)/*.elf)
ALLUSER_TASK_OBJS += $(addprefix $(GZ_APP_OUT)/,$(GZ_USER_TASK_LIST))

OUTELF := $(BUILDROOT)/lk.elf
OUTBIN := $(BUILDROOT)/lk.bin
OUTENC := $(BUILDROOT)/lk.bin.enc
ifeq ($(strip $(MTK_NEBULA_VM_SUPPORT)),yes)
VM_OUTELF := $(BUILDROOT)/vm.ori
VM_OUTBIN := $(BUILDROOT)/vm.bin
VM_OUTENC := $(BUILDROOT)/vm.bin.enc
VM_OUTPROCESSED := $(BUILDROOT)/vm.bin.processed
VM_PREBUILTS_PATH = $(PREBUILTS_PATH)/vm/nebula
VM_PREBUILTS_FILE = $(VM_PREBUILTS_PATH)/nebula_hee.bin

$(VM_OUTELF): $(VM_PREBUILTS_FILE)
	@echo copying vm binary $@
	$(hide) cp $(VM_PREBUILTS_FILE) $(VM_OUTELF)

$(VM_OUTBIN): ALIGNMENT := 16
$(VM_OUTBIN): $(VM_OUTELF)
	@echo generating vm.bin: $@
	$(hide) cp $(VM_OUTELF) $(VM_OUTBIN)
	$(hide) FILE_SIZE=$$(wc -c < "$(VM_OUTELF)");\
        REMAINDER=$$(($${FILE_SIZE} % $(ALIGNMENT)));\
        if [ $${REMAINDER} -ne 0 ]; then dd if=/dev/zero bs=$$(($(ALIGNMENT)-$${REMAINDER})) count=1 >> $@; else touch $@; fi

$(VM_OUTENC): PLATFORM_K_VALUE := $(shell cat $(GZ_KEY_PATH)/$(GZ_TARGET_BOARD_PLATFORM).k)
$(VM_OUTENC): PLATFORM_IV_VALUE := $(shell cat $(GZ_KEY_PATH)/$(GZ_TARGET_BOARD_PLATFORM).iv)
$(VM_OUTENC): $(VM_OUTBIN)
	@echo Encrypt vm.bin: $@
	$(hide) openssl enc -aes-128-ctr -in $< -out $@ -K $(PLATFORM_K_VALUE) -iv $(PLATFORM_IV_VALUE)
ifeq ($(strip $(GZ_ENC)),1)
	$(hide) cp $(VM_OUTENC) $(VM_OUTPROCESSED)
else
	$(hide) cp $(VM_OUTBIN) $(VM_OUTPROCESSED)
endif
endif

#ifneq ($(strip $(MTK_NEBULA_VM_SUPPORT)),yes)
$(USER_TASKS_BIN): $(ALLUSER_TASK_OBJS)
	@echo combining tasks into $@: $(ALLUSER_TASK_OBJS)
	$(hide) mkdir -p $(BUILDROOT)
	$(hide) cat $(ALLUSER_TASK_OBJS) > $@

$(USER_TASKS_OBJ): $(USER_TASKS_BIN)
	@echo generating $@
	$(hide) $(GZ_LD) -r -b binary -o $@ $<
	$(hide) $(GZ_OBJCOPY) --prefix-sections=.task $@



$(KERNEL_PREBUILT_OBJ): $(KERNEL_PREBUILT_ARCHIVE)
	@echo extracting $<
	$(hide) mkdir -p $(BUILDROOT)/kernel/
	$(hide) cd $(BUILDROOT)/kernel/ && $(GZ_AR) x $(KERNEL_PREBUILT_ARCHIVE)

$(OUTELF): $(USER_TASKS_OBJ) $(KERNEL_PREBUILT_OBJ)
	@echo linking $@
	$(hide) $(GZ_LD) -L$(PREBUILTS_LDSCRIPTS) --gc-sections -T $(LK_LDSCRIPTS) \
	--start-group \
	$(KERNEL_PREBUILT_OBJS) ${USER_TASKS_OBJ} \
	--end-group \
	$(LIBGCC) -o $@

# Make 16-bytes alignment for lk.bin
$(OUTBIN): ALIGNMENT := 16
$(OUTBIN): $(OUTELF)
	@echo generating lk.bin: $@
	$(hide) $(GZ_OBJCOPY) -O binary $< $@
	$(hide) FILE_SIZE=$$(wc -c < "$(OUTBIN)");\
        REMAINDER=$$(($${FILE_SIZE} % $(ALIGNMENT)));\
        if [ $${REMAINDER} -ne 0 ]; then dd if=/dev/zero bs=$$(($(ALIGNMENT)-$${REMAINDER})) count=1 >> $@; else touch $@; fi

$(OUTENC): PLATFORM_K_VALUE := $(shell cat $(GZ_KEY_PATH)/$(GZ_TARGET_BOARD_PLATFORM).k)
$(OUTENC): PLATFORM_IV_VALUE := $(shell cat $(GZ_KEY_PATH)/$(GZ_TARGET_BOARD_PLATFORM).iv)
$(OUTENC): $(OUTBIN)
ifeq ($(strip $(GZ_ENC)),1)
	@echo Encrypt lk.bin: $@
	$(hide) openssl enc -aes-128-ctr -in $< -out $@ -K $(PLATFORM_K_VALUE) -iv $(PLATFORM_IV_VALUE)
else
	@echo GZ encryption is disabled!
	$(hide) cp $< $@
endif

.PHONY: $(GZ_IMAGE)
$(GZ_IMAGE): $(GZ_INSTALLED_TARGET) $(GZ_INSTALLED_TARGET_IMG) $(GZ_SIGN_IMAGE_TARGET)
$(GZ_SIGN_IMAGE_TARGET):

include $(CLEAR_VARS)
LOCAL_MODULE := $(GZ_TARGET_FILE)
LOCAL_MODULE_CLASS := ETC
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_PATH := $(PRODUCT_OUT)
include $(BUILD_SYSTEM)/base_rules.mk

ifeq ($(strip $(MTK_NEBULA_VM_SUPPORT)),yes)
$(LOCAL_BUILT_MODULE): $(VM_OUTENC)
endif

$(LOCAL_BUILT_MODULE): $(OUTENC)
	@echo generating gz image: $@
	$(hide) echo "LOAD_MODE = 0" > $(BUILDROOT)/img_hdr_lk.tmp
	$(hide) echo "NAME = gz" >> $(BUILDROOT)/img_hdr_lk.tmp
	$(hide) cat $(BUILDROOT)/img_hdr_lk.tmp $(GZ_ROOT_DIR)/trusty/device/arm/mediatek/build/gz_info.cfg > $(BUILDROOT)/img_hdr_lk.cfg
	$(hide) rm $(BUILDROOT)/img_hdr_lk.tmp
	$(hide) rm -f $(GZ_INSTALLED_TARGET)
ifeq ($(strip $(GZ_ENC)),1)
	$(hide) $(MKIMAGE) $(BUILDROOT)/lk.bin.enc  $(BUILDROOT)/img_hdr_lk.cfg > $(BUILDROOT)/lk.bin.img
endif
ifeq ($(strip $(GZ_ENC)),0)
	$(hide) $(MKIMAGE) $(BUILDROOT)/lk.bin  $(BUILDROOT)/img_hdr_lk.cfg > $(BUILDROOT)/lk.bin.img
endif
	$(hide) cat $(BUILDROOT)/lk.bin.img > $@
ifeq ($(strip $(MTK_NEBULA_VM_SUPPORT)),yes)
	@echo generating vm image..
	$(hide) echo "LOAD_MODE = 0" > $(BUILDROOT)/vm_info.cfg
	$(hide) echo "NAME = vm" >> $(BUILDROOT)/vm_info.cfg
	$(hide) echo "LOAD_ADDR = 0xFFFFFFFF" >> $(BUILDROOT)/vm_info.cfg
	$(hide) $(MKIMAGE) $(VM_OUTPROCESSED) $(BUILDROOT)/vm_info.cfg > $(BUILDROOT)/vm.bin.img
	$(hide) cat $(BUILDROOT)/vm.bin.img >> $@
endif

include $(CLEAR_VARS)
LOCAL_MODULE := $(GZ_TARGET_FILE_IMG)
LOCAL_MODULE_CLASS := ETC
LOCAL_MULTILIB := 32
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_PATH := $(PRODUCT_OUT)
include $(BUILD_SYSTEM)/base_rules.mk

$(LOCAL_BUILT_MODULE): $(GZ_INSTALLED_TARGET)
	$(hide) $(copy-file-to-target)

.PHONY: $(GZ_IMAGE_CLEAN)
$(GZ_IMAGE_CLEAN):
	@echo "Clean: $(GZ_IMAGE_CLEAN)"
	$(hide) rm -rf $(BUILDROOT)/*

endif # MTK_ENABLE_GENIEZONE

