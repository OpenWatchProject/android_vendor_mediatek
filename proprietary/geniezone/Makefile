V ?= 0

#
#  Directory seeting
#

PLATFORM ?= unknown

TOOLCHAIN_PATH ?= $(ANDROID_BUILD_TOP)/prebuilts/gcc
MTKTOOLAS_PATH ?= $(PWD)/scripts
BUILDROOT ?= $(PWD)
BUILDPROJECT=${BUILDROOT}/build-${PROJECT}


#
# Full Build Setting
# 2018/03/12 add for release all gz, except some properity trusty-kernel module
#

FULL_BUILD ?= 0
PRIVATE_PREBUILTS_PATH = $(PWD)/trusty/vendor/mediatek/geniezone/prebuilts/$(PROJECT)
export FULL_BUILD
export PRIVATE_PREBUILTS_PATH

#
# Guest Build Setting
#

BUILD_VM ?= 0
VM_PREBUILTS_INTERNAL_PATH = $(BUILDROOT)/prebuilts/third-party/vm/nebula
VM_PREBUILTS_PATH = $(BUILDROOT)/prebuilts/libs/vm/nebula
VM_PREBUILTS_FILE = $(VM_PREBUILTS_PATH)/nebula_hee.bin
VM_IV_FILE = $(BUILDROOT)/trusty/device/arm/mediatek/project/iv/$(PLATFORM)/$(PLATFORM).iv
VM_KEY_FILE = $(BUILDROOT)/trusty/device/arm/mediatek/project/iv/$(PLATFORM)/$(PLATFORM).k
export BUILD_VM
export VM_PREBUILTS_PATH
export VM_PREBUILTS_FILE
export VM_IV_FILE
export VM_KEY_FILE

#
# Prebuilt Settings
#

WITH_PREBUILT_LIB ?= 0

PREBUILTS_PATH=$(PWD)/prebuilts/libs
PREBUILTS_LIB_PATH=${PREBUILTS_PATH}/$(PROJECT)/lib
PREBUILTS_LDSCRIPTS_PATH=${PREBUILTS_PATH}/$(PROJECT)/ldscripts
PREBUILTS_KERNEL_PATH=${PREBUILTS_PATH}/$(PROJECT)/kernel
PREBUILTS_APP_PATH=${PREBUILTS_PATH}/$(PROJECT)/app

#
# Common functions calls
#

mkdir_host = mkdir -p $(1);
cp_host = cp -f $1 $2;

# Copy all propietary apps

list_all_app = $(notdir $(wildcard $(BUILDPROJECT)/user_tasks/source/trusty-user/app/*))
find_app_elf = $(shell find $(BUILDPROJECT)/user_tasks/source/trusty-user/app/$(strip ${1}) -name "*.elf" ! -name "*.syms.elf")

copy_app_prebuilt = \
        $(call cp_host, $(call find_app_elf, $(strip $1)), $(PREBUILTS_APP_PATH)/$(strip $1).elf)

copy_all_app_prebuilt = $(foreach ut, $(call list_all_app), $(call copy_app_prebuilt, $(strip $(ut))))

# Copy ldscripts
list_all_ldscripts = $(wildcard $(BUILDPROJECT)/system-onesegment.ld)
copy_all_ldscripts = $(call cp_host, $(list_all_ldscripts), $(PREBUILTS_LDSCRIPTS_PATH)/.)

# Copy kernel lib
list_all_kernel_lib = $(wildcard $(BUILDPROJECT)/external/lk/lib/libc.mod.a)
copy_all_kernel_lib = $(call cp_host, $(list_all_kernel_lib), $(PREBUILTS_LIB_PATH)/arm64/.)

# Copy all prebuilt user lib

list_all_user_lib = libc.mod.a libc-trusty.mod.a libstdc++-trusty.mod.a libm-mtee.mod.a libcxx-mtee.mod.a crtbegin.o crtend.o mtee_api.mod.a mtee_serv.mod.a msee_fwk.mod.a gp_client.mod.a storage.mod.a

find_user_lib_elf = $(word 1, $(shell find $(BUILDPROJECT)/prebuilts/user-lib-prebuilt/ -name $1))

copy_lib_prebuilt = \
	$(call cp_host, $(call find_user_lib_elf, $(strip $1)), $(PREBUILTS_LIB_PATH)/arm/.)

copy_all_user_lib = $(foreach lib, $(list_all_user_lib), $(call copy_lib_prebuilt, $(strip $(lib))))


# Copy all prebuilt kernel objs

list_all_kernel_objs = kernel-prebuilt.a
find_kernel_objs_elf = $(shell find $(BUILDPROJECT)/ -name $1)

copy_kernel_objs_prebuilt = \
        $(call cp_host, $(call find_kernel_objs_elf, $(strip $(list_all_kernel_objs))), $(PREBUILTS_KERNEL_PATH)/$(PROJECT)-kernel.a)

# Prebuilt toolchain setting
ARCH_arm64_TOOLCHAIN_PREFIX=${TOOLCHAIN_PATH}/linux-x86/aarch64/aarch64-linux-android-4.9/bin/aarch64-linux-android-

GCC4P8 ?= 0
export GCC4P8
ifneq ($(GCC4P8), 1)
ARCH_arm_TOOLCHAIN_PREFIX=${TOOLCHAIN_PATH}/linux-x86/arm/arm-eabi-4.9/bin/arm-eabi-
ARCH_arm_TOOLCHAIN_LD_LIBRARY_PATH=LD_LIBRARY_PATH=${TOOLCHAIN_PATH}/linux-x86/arm/arm-eabi-4.9/libexec/gcc/arm-eabi/4.9.4/libmpfr
else
ARCH_arm_TOOLCHAIN_PREFIX=${TOOLCHAIN_PATH}/linux-x86/arm/arm-eabi-4.8/bin/arm-eabi-
endif
export ARCH_arm_TOOLCHAIN_PREFIX ARCH_arm64_TOOLCHAIN_PREFIX ARCH_arm_TOOLCHAIN_LD_LIBRARY_PATH

CORE_NUM ?= 0
export CORE_NUM
GZ_DEBUG ?= 0
export GZ_DEBUG
ifeq ($(GZ_DEBUG), 1)
GZ_LOG_LVL ?= 1
else
GZ_LOG_LVL ?= 4
endif
export GZ_LOG_LVL
VPU ?= mt6779
VPU_ELF = ${VPU}_vpu
export VPU_ELF

# Verbose ouput
ifeq ($(V),1)
	ADDITIONAL_FLAGS += "NOECHO="
endif

all:
	make -C trusty ${PROJECT} BUILDROOT=$(BUILDROOT) PREBUILTS_PATH=$(PREBUILTS_PATH) WITH_PREBUILT_LIB=$(WITH_PREBUILT_LIB)  $(ADDITIONAL_FLAGS)
ifneq ($(PLATFORM),unknown)
	cp $(BUILDROOT)/trusty/vendor/mediatek/geniezone/platform/$(PLATFORM)/gz_info.cfg $(BUILDPROJECT)
endif
	${MTKTOOLAS_PATH}/build_hyp.sh $(BUILDPROJECT)/lk.bin
	ls -l $(BUILDPROJECT)/gz.bin
ifeq ($(FULL_BUILD), 1)
	mkdir -p $(PRIVATE_PREBUILTS_PATH)/trusty-kernel
	mkdir -p $(PRIVATE_PREBUILTS_PATH)/trusty-kernel/mtee/common/src/
	cp $(BUILDPROJECT)/vendor/mediatek/private/source/trusty-kernel/libvmm.mod.o $(PRIVATE_PREBUILTS_PATH)/trusty-kernel/libvmm.mod.o
	cp $(BUILDPROJECT)/vendor/mediatek/geniezone/source/trusty-kernel/libfdt.mod.o $(PRIVATE_PREBUILTS_PATH)/trusty-kernel/libfdt.mod.o
	cp $(BUILDPROJECT)/vendor/mediatek/private/source/trusty-kernel/mtee/common/src/sys.mod.o $(PRIVATE_PREBUILTS_PATH)/trusty-kernel/mtee/common/src/sys.mod.o
endif
ifeq ($(BUILD_VM), 1)
	mv $(BUILDPROJECT)/gz.bin $(BUILDPROJECT)/gz.bin.ori
	mv $(BUILDPROJECT)/gz.img $(BUILDPROJECT)/gz.img.ori
	cp $(BUILDROOT)/trusty/vendor/mediatek/geniezone/platform/$(PLATFORM)/vm_info.cfg $(BUILDPROJECT)
	cp $(VM_PREBUILTS_INTERNAL_PATH)/nebula_hee.bin  $(VM_PREBUILTS_FILE)
	cp $(VM_PREBUILTS_FILE) $(BUILDPROJECT)/vm.ori
	${MTKTOOLAS_PATH}/build_vm.sh $(BUILDPROJECT)/vm.ori $(WITH_IMAGE_ENCRYPT) $(VM_IV_FILE) $(VM_KEY_FILE)
	ls -l $(BUILDPROJECT)/vm.bin
	cat $(BUILDPROJECT)/gz.bin.ori > $(BUILDPROJECT)/gz.bin
	cat $(BUILDPROJECT)/vm.bin >> $(BUILDPROJECT)/gz.bin
	ls -l $(BUILDPROJECT)/gz.bin
endif

release_lib:
	$(call mkdir_host,${PREBUILTS_LIB_PATH}/arm)
	$(if $(strip $(wildcard $(BUILDPROJECT))), $(call copy_all_user_lib), $(error please `make all` first))
	$(call mkdir_host,${PREBUILTS_LIB_PATH}/arm64)
	$(call copy_all_kernel_lib)

release_app:
ifneq (,$(wildcard $(BUILDPROJECT)/user_tasks/gz-test/gz-test.elf))
	cp $(BUILDPROJECT)/user_tasks/gz-test/gz-test.elf $(PREBUILTS_APP_PATH)
endif
	$(call mkdir_host,$(PREBUILTS_APP_PATH))
	$(if $(strip $(wildcard $(BUILDPROJECT))), $(call copy_all_app_prebuilt), $(error please `make all` first))


release_kernel:
	$(call mkdir_host,$(PREBUILTS_KERNEL_PATH))
	$(if $(strip $(wildcard $(BUILDPROJECT))), $(call copy_kernel_objs_prebuilt), $(error please `make all` first))
	$(call mkdir_host,$(PREBUILTS_LDSCRIPTS_PATH))
	$(copy_all_ldscripts)

release_all: release_kernel release_app release_lib
