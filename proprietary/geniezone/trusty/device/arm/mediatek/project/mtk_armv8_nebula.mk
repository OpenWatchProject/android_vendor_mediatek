#
# Copyright (C) 2015 MediaTek Inc.
#
# Modification based on code covered by the below mentioned copyright
# and/or permission notice(S).
#

# Copyright (C) 2015 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_DIR := $(GET_LOCAL_DIR)

DEBUG ?= 2
SMP_MAX_CPUS ?= 8
SMP_CPU_CLUSTER_SHIFT ?= 2

TARGET := mtk_armv8_nebula

ifeq (false,$(call TOBOOL,$(KERNEL_32BIT)))

# Arm64 address space configuration
KERNEL_ASPACE_BASE := 0x000001000
KERNEL_ASPACE_SIZE := 0x1000000000
KERNEL_BASE        := 0x000000000

GLOBAL_DEFINES += MMU_USER_SIZE_SHIFT=28 # 256 MB user-space address space
GLOBAL_DEFINES += USER_ASPACE_SIZE=0x8000000 # 128MB

else

#################################
# Add project specific defines here w/ ':=' to override platform defaults
#################################
#KERNEL_BASE        := 0x4f040000
#MEM_SIZE           := 0x500000
#CFG_LOG_BAUDRATE   := 921600
#CFG_LOG_REG_BASE   := UART0_BASE # check reg_base.h for UARTX_BASE details

endif

WITH_MTEE := 1
# mtee framework support
GLOBAL_DEFINES += WITH_MTEE=$(WITH_MTEE)

WITH_MBLOCK := 1
# make mblock config available to C sources
GLOBAL_DEFINES += WITH_MBLOCK=$(WITH_MBLOCK)

WITH_NEW_DT := 1
# make new device tree config available to C sources
GLOBAL_DEFINES += WITH_NEW_DT=$(WITH_NEW_DT)

WITH_CPU_SUSPEND := 1
# enable ACAO CPU suspend support
GLOBAL_DEFINES += WITH_CPU_SUSPEND=$(WITH_CPU_SUSPEND)

WITH_LK_GUEST := 1
# enable 32-bit LK guest support
GLOBAL_DEFINES += WITH_LK_GUEST=$(WITH_LK_GUEST)

# WITH_HTEE_SUPPORT is used for backward compatible after GZ 3.3
WITH_HTEE_SUPPORT := 1
# enable HTEE guest support
GLOBAL_DEFINES += WITH_HTEE_SUPPORT=$(WITH_HTEE_SUPPORT)

WITH_ARCH_TIMER_BACKUP_RESTORE := 1
# enable arch_timer backup/restore
GLOBAL_DEFINES += WITH_ARCH_TIMER_BACKUP_RESTORE=$(WITH_ARCH_TIMER_BACKUP_RESTORE)

WITH_VMM_SWITCH_EVENTS := 0
# enable vmm enter/exist notify calls

WITH_VMM_FAKE_MPIDR := 0
# enable vmm fake mpidr with legacy format

WITH_SIP_SMC_ID_REMAKE_SUPPORT := 1
GLOBAL_DEFINES += WITH_SIP_SMC_ID_REMAKE_SUPPORT=$(WITH_SIP_SMC_ID_REMAKE_SUPPORT)
# enable SIP SMC ID[15:14] re-make for permission check in ATF
# Linux [15:14] will be changed from 0b00 to 0b01, then to ATF
# Linux [15:14] != 0b00 will be forbidden
# GZ SIP call [15:14] will be changed from 0b00 to 0b10, then to ATF

WITH_HAPP_THIN_EL1_SUPPORT := 0
# enable thin EL1 layer for HAPP
GLOBAL_DEFINES += WITH_HAPP_THIN_EL1_SUPPORT=$(WITH_HAPP_THIN_EL1_SUPPORT)

WITH_MBLOCK_RELOAD_SUPPORT := 1
# reload mblock memory info before jumping to Linux kernel
GLOBAL_DEFINES += WITH_MBLOCK_RELOAD_SUPPORT=$(WITH_MBLOCK_RELOAD_SUPPORT)

GLOBAL_DEFINES += WITH_VMM_SWITCH_EVENTS=1
GLOBAL_DEFINES += WITH_VMM_FAKE_MPIDR=$(WITH_VMM_FAKE_MPIDR)

WITH_HA_DYNAMIC_LOADING := 0
# enable HA dynamic loading
GLOBAL_DEFINES += WITH_HA_DYNAMIC_LOADING=$(WITH_HA_DYNAMIC_LOADING)

# GenieZone related common configs
include project/gz_common.mk

GLOBAL_DEFINES += GZ_MULTI_THREAD
#GLOBAL_DEFINES += GZ_MULTI_THREAD_DEBUG

GZ_SECURE_RTC := 1
# enable secure RTC
GLOBAL_DEFINES += GZ_SECURE_RTC=$(GZ_SECURE_RTC)

CFG_GZ_SECURE_DSP := 1
# enable secure DSP
GLOBAL_DEFINES += CFG_GZ_SECURE_DSP=$(CFG_GZ_SECURE_DSP)

CFG_LOG_WORKAROUND := 0
# enable multicore log workaround
GLOBAL_DEFINES += CFG_LOG_WORKAROUND=$(CFG_LOG_WORKAROUND)

CFG_GZ_SECURE_DSP_WITH_M4U := 1
# enable secure DSP
GLOBAL_DEFINES += CFG_GZ_SECURE_DSP_WITH_M4U=$(CFG_GZ_SECURE_DSP_WITH_M4U)

##################################
# Override GZ configs here
#################################
# KTEE/MTEE unitest
#WITH_MTEE_KERNEL_UNITTEST := 1
#WITH_MTEE_USER_UNITTEST := 1

# GenieZone common tasks
include project/gz_common_task.mk

EXTRA_BUILDRULES += app/trusty/user-tasks.mk
