###################################################################
# Global feature options that need to align with Android ones
###################################################################
CFG_AUDIO_SUPPORT = yes
CFG_MTK_AUDIO_TUNNELING_SUPPORT = yes
CFG_MTK_AURISYS_PHONE_CALL_SUPPORT = no
CFG_MTK_VOW_SUPPORT = no

###################################################################
# ADSP release policy
###################################################################
CFG_3RDPARTY_PROJECT = yes

###################################################################
# ADSP internal feature options
###################################################################
CFG_ASSERT_SUPPORT = yes
CFG_XGPT_SUPPORT = yes
CFG_UART_SUPPORT = no
CFG_TRAX_SUPPORT = no
CFG_TASK_MONITOR = yes
# CFG_MTK_APUART_SUPPORT
# Do not use this with eng load or log may mix together and hard to recognzie
# Do not use this on lower power, it keeps infra always on
CFG_CACHE_SUPPORT = yes
CFG_SEM_SUPPORT = yes
CFG_IPC_SUPPORT = yes
CFG_KERNEL_TIMESTAMP_SUPPORT = yes
CFG_ADSP_LOGGER_SUPPORT = yes
CFG_WDT_SUPPORT = yes
CFG_DMA_SUPPORT = yes
CFG_RECOVERY_SUPPORT = no
CFG_VCORE_DVFS_SUPPORT = yes
CFG_MPU_SUPPORT = yes
CFG_RAMDUMP_SUPPORT = no
CFG_TICKLESS_SUPPORT = yes
CFG_FREERTOS_TRACE_SUPPORT = no
CFG_IRQ_MONITOR_SUPPORT = yes
CFG_WAKELOCK_SUPPORT = yes
CFG_MTK_AUDIO_FRAMEWORK_SUPPORT = yes
CFG_MTK_AURISYS_FRAMEWORK_SUPPORT = yes
CFG_UT_TESTSUITE_SUPPORT = no
CFG_SYSTEM_OFF_SUPPORT = yes
CFG_PERFORMANCE_MONITOR_SUPPORT = yes
CFG_MCPS_PROF_SUPPORT = no
CFG_A2DP_OFFLOAD_SUPPORT = no
CFG_BUS_MONITOR_SUPPORT = yes
CFG_DEBUG_COMMAND_SUPPORT = yes

CFG_HIFI3_IRAM_ADDRESS = 0x4ffe0000
CFG_HIFI3_DRAM_ADDRESS = 0x4ffb0000
CFG_HIFI3_SRAM_ADDRESS = 0x55c00000
CFG_AUDIO_BASE_ADDRESS = 0xD1210000
CFG_HIFI3_IRAM_SIZE    = 0x9000
CFG_HIFI3_DRAM_SIZE    = 0x8000
CFG_HIFI3_SRAM_SIZE    = 0x700000
CFG_AUDIO_CONFIG_SIZE  = 0x1000
CFG_AUDIO_DATA_SIZE    = 0x18000

###################################################################
# DEBUG_LEVEL
###################################################################
CFLAGS += -DDEBUGLEVEL=3

###################################################################
# HW change
###################################################################

###################################################################
# Optional ProjectConfig.mk used by project
###################################################################
-include $(PROJECT_DIR)/ProjectConfig.mk

###################################################################
# Mandatory platform-specific resources
###################################################################
XT_TOOLS_VERSION = RI-2018.0-linux
TCM_LAYOUT_VERSION = 2
DRIVERS_PLATFORM_DIR_COMMON = $(DRIVERS_DIR)/$(PROCESSOR)/mt6779

INCLUDES += \
  $(PLATFORM_DIR)/inc \
  $(DRIVERS_PLATFORM_DIR)/feature_manager/inc

C_FILES += \
  $(PLATFORM_DIR)/src/main.c \
  $(PLATFORM_DIR)/src/platform.c \
  $(DRIVERS_PLATFORM_DIR)/feature_manager/src/feature_manager.c

###################################################################
# Heap size config
###################################################################
# default heap size
CFLAGS += -DconfigTOTAL_HEAP_SIZE='( ( size_t ) ( 3 * 1024 * 1024 ) )'

###################################################################
# Resources determined by configuration options
###################################################################
ifeq ($(CFG_CACHE_SUPPORT), yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/cache/inc
C_FILES  += $(DRIVERS_PLATFORM_DIR)/cache/src/hal_cache.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/cache/src/hal_cache_internal.c
endif

ifeq ($(CFG_FREERTOS_TRACE_SUPPORT),yes)
C_FILES  += middleware/ondiemet/met_freertos.c
C_FILES  += middleware/ondiemet/met_main.c
C_FILES  += middleware/ondiemet/met_util.c
C_FILES  += middleware/ondiemet/met_tag.c
C_FILES  += middleware/ondiemet/met_log.c
INCLUDES += middleware/ondiemet/inc
CFLAGS  += -DMET_TAG
endif

ifeq ($(CFG_VCORE_DVFS_SUPPORT),yes)
#C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_dvfs/src/dvfs_test.c
endif

ifeq ($(CFG_XGPT_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/xgpt/inc
C_FILES  += $(DRIVERS_PLATFORM_DIR)/xgpt/src/xgpt.c
endif

ifeq ($(CFG_UART_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/uart/inc
C_FILES  += $(SOURCE_DIR)/drivers/common/uart/v01/uart.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/uart/src/uart_platform.c
endif

ifeq ($(CFG_TRAX_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/trax/inc
C_FILES  += $(DRIVERS_PLATFORM_DIR)/trax/src/adsp_trax.c
endif

ifeq ($(CFG_SEM_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/sem/inc
C_FILES  += $(SOURCE_DIR)/drivers/common/sem/v02/src/scp_sem.c
endif

ifeq ($(CFG_IPC_SUPPORT),yes)
INCLUDES += $(SOURCE_DIR)/drivers/common/ipi/common
C_FILES  += $(SOURCE_DIR)/drivers/common/ipi/common/adsp_ipi_queue.c
INCLUDES += $(DRIVERS_PLATFORM_DIR)/ipi/inc
C_FILES  += $(DRIVERS_PLATFORM_DIR)/ipi/src/adsp_ipi.c
endif

ifeq ($(CFG_ADSP_LOGGER_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/logger/inc
C_FILES  += $(DRIVERS_PLATFORM_DIR)/logger/src/adsp_logger.c
endif

ifeq ($(CFG_WDT_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/wdt/inc
C_FILES  += $(DRIVERS_PLATFORM_DIR)/wdt/src/wdt.c
endif

ifeq ($(CFG_DMA_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/dma/inc
C_FILES  += $(DRIVERS_PLATFORM_DIR)/dma/src/dma.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/dma/src/dma_api.c
endif

ifeq ($(CFG_WAKELOCK_SUPPORT),yes)
INCLUDES += $(SOURCE_DIR)/kernel/FreeRTOS/Source/include
INCLUDES += $(SOURCE_DIR)/kernel/service/HIFI3/include
C_FILES  += $(SOURCE_DIR)/kernel/service/HIFI3/src/wakelock.c
INCLUDES += $(DRIVERS_PLATFORM_DIR)/ap_awake/inc
C_FILES  += $(DRIVERS_PLATFORM_DIR)/ap_awake/src/ap_awake.c
endif

ifeq ($(CFG_DEBUG_COMMAND_SUPPORT),yes)
INCLUDES += $(SOURCE_DIR)/kernel/service/HIFI3/include
C_FILES  += $(SOURCE_DIR)/kernel/service/HIFI3/src/adsp_dbg.c
endif

ifeq ($(CFG_BUS_MONITOR_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/bus_monitor/inc
C_FILES  += $(DRIVERS_PLATFORM_DIR)/bus_monitor/src/bus_monitor.c
endif

ifeq ($(CFG_UT_TESTSUITE_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/tests/inc
C_FILES  += $(DRIVERS_PLATFORM_DIR)/tests/src/test_main.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/tests/src/testsuite_dma.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/tests/src/testsuite_wdt.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/tests/src/testsuite_dvfs.c
endif

ifeq ($(CFG_HEAP_GUARD_SUPPORT),yes)
C_FILES  += $(RTOS_SRC_DIR)/portable/MemMang/mtk_HeapGuard.c
LDFLAGS += -Wl, -wrap=pvPortMalloc -Wl, -wrap=vPortFree
endif

ifeq ($(CFG_VCORE_DVFS_SUPPORT),yes)
#INCLUDES += $(SOURCE_DIR)/kernel/service/common/include/
INCLUDES += $(DRIVERS_PLATFORM_DIR)/dvfs/inc
C_FILES  += $(DRIVERS_PLATFORM_DIR)/dvfs/src/dvfs_common.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/dvfs/src/dvfs.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/dvfs/src/dvfs_config_parser.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/dvfs/src/sleep.c
endif

ifeq ($(CFG_PERFORMANCE_MONITOR_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/perf_mon/inc
C_FILES  += $(DRIVERS_PLATFORM_DIR)/perf_mon/src/perf_mon.c
endif

ifeq ($(CFG_MCPS_PROF_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/mcps_prof/inc
C_FILES  += $(DRIVERS_PLATFORM_DIR)/mcps_prof/src/mcps_prof.c
CFLAGS  += -DMCPS_PROF_SUPPORT
endif

ifeq ($(CFG_MPU_SUPPORT),yes)
INCLUDES += $(SOURCE_DIR)/kernel/service/HIFI3/include
INCLUDES += $(DRIVERS_PLATFORM_DIR)/mpu/inc
C_FILES  += $(DRIVERS_PLATFORM_DIR)/mpu/src/mpu_mtk.c
endif

# offload playback
ifeq ($(CFG_MTK_AUDIO_TUNNELING_SUPPORT),yes)
CFG_AUDIO_SUPPORT = yes
endif

ifeq ($(CFG_MTK_VOW_SUPPORT),yes)
CFG_AUDIO_SUPPORT = yes
endif


# hifi playback
ifeq ($(CFG_MTK_AUDIO_FRAMEWORK_SUPPORT),yes)
CFG_AUDIO_SUPPORT = yes
endif

ifeq ($(CFG_AUDIO_SUPPORT),yes)
INCLUDES += $(SOURCE_DIR)/middleware/lib/aurisys/interface
INCLUDES += $(SOURCE_DIR)/middleware/lib/audio_utility
INCLUDES += $(SOURCE_DIR)/drivers/common/audio/framework
INCLUDES += $(SOURCE_DIR)/drivers/common/audio/hardware
INCLUDES += $(SOURCE_DIR)/drivers/common/audio/tasks
INCLUDES += $(DRIVERS_PLATFORM_DIR)/audio/hardware
C_FILES  += $(SOURCE_DIR)/middleware/lib/audio_utility/audio_ringbuf.c
C_FILES  += $(SOURCE_DIR)/drivers/common/audio/framework/audio.c
C_FILES  += $(SOURCE_DIR)/drivers/common/audio/framework/audio_ipi_dma.c
C_FILES  += $(SOURCE_DIR)/drivers/common/audio/framework/audio_messenger_ipi.c
C_FILES  += $(SOURCE_DIR)/drivers/common/audio/framework/audio_task_factory.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/audio/hardware/audio_hw.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/audio/hardware/audio_irq.c
C_FILES  += $(SOURCE_DIR)/drivers/common/audio/framework/audio_task_utility.c
C_FILES  += $(SOURCE_DIR)/drivers/common/audio/framework/audio_dsp_hw_hal.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/audio/hardware/audio_hw_hal.c


INCLUDES += $(SOURCE_DIR)/drivers/common/audio/tasks/controller
C_FILES  += $(SOURCE_DIR)/drivers/common/audio/tasks/controller/audio_task_controller.c


### utility
INCLUDES += $(SOURCE_DIR)/middleware/lib/audio/hifi3/blisrc
INCLUDES += $(SOURCE_DIR)/middleware/lib/audio/hifi3/shifter

LIBFLAGS += \
  -Wl,-L$(SOURCE_DIR)/middleware/lib/audio/hifi3/blisrc,-lblisrc \
  -Wl,-L$(SOURCE_DIR)/middleware/lib/audio/hifi3/shifter,-lshifter

CFLAGS += -DMTK_AUDIO_LOCK_ENABLE_TRACE
#CFLAGS += -DMTK_AUDIO_LOCK_ENABLE_LOG

C_FILES  += $(SOURCE_DIR)/middleware/lib/audio_utility/audio_memory_control.c
C_FILES  += $(SOURCE_DIR)/middleware/lib/audio_utility/audio_lock.c
C_FILES  += $(SOURCE_DIR)/middleware/lib/audio_utility/audio_ringbuf_pure.c
C_FILES  += $(SOURCE_DIR)/middleware/lib/audio_utility/audio_sample_rate.c
C_FILES  += $(SOURCE_DIR)/middleware/lib/audio_utility/audio_fmt_conv.c
endif

ifeq ($(CFG_MTK_AURISYS_PHONE_CALL_SUPPORT),yes)
C_FILES  += $(SOURCE_DIR)/drivers/common/audio/tasks/call/audio_task_phone_call.c
INCLUDES += $(SOURCE_DIR)/drivers/common/audio/tasks/call
endif

### ============================================================================
### Aurisys Framework
### ============================================================================

ifeq ($(CFG_MTK_AURISYS_FRAMEWORK_SUPPORT),yes)
CFLAGS += -DMTK_AURISYS_FRAMEWORK_SUPPORT
#CFLAGS += -DAURISYS_BYPASS_ALL_LIBRARY
#CFLAGS += -DAURISYS_DUMP_LOG_V

INCLUDES += $(SOURCE_DIR)/middleware/lib/audio_utility/uthash

INCLUDES += $(SOURCE_DIR)/middleware/lib/aurisys/interface
INCLUDES += $(SOURCE_DIR)/middleware/lib/aurisys/utility
INCLUDES += $(SOURCE_DIR)/middleware/lib/aurisys/framework

C_FILES  += $(SOURCE_DIR)/middleware/lib/aurisys/utility/aurisys_utility.c
C_FILES  += $(SOURCE_DIR)/middleware/lib/aurisys/utility/aurisys_adb_command.c
C_FILES  += $(SOURCE_DIR)/middleware/lib/aurisys/utility/audio_pool_buf_handler.c

C_FILES  += $(SOURCE_DIR)/middleware/lib/aurisys/framework/aurisys_config_parser.c
C_FILES  += $(SOURCE_DIR)/middleware/lib/aurisys/framework/aurisys_controller.c
C_FILES  += $(SOURCE_DIR)/middleware/lib/aurisys/framework/aurisys_lib_manager.c
C_FILES  += $(SOURCE_DIR)/middleware/lib/aurisys/framework/aurisys_lib_handler.c

C_FILES  += $(SOURCE_DIR)/drivers/common/audio/tasks/common/audio_task_aurisys.c


INCLUDES += $(SOURCE_DIR)/middleware/lib/aurisys/lib/common/libaurisysdemo
C_FILES  += $(SOURCE_DIR)/middleware/lib/aurisys/lib/common/libaurisysdemo/aurisys_demo_lib.c
INCLUDES += $(SOURCE_DIR)/middleware/lib/audio/hifi3/libbesloudness
INCLUDES += $(SOURCE_DIR)/middleware/lib/aurisys/lib/common/libbesloudness
C_FILES  += $(SOURCE_DIR)/middleware/lib/aurisys/lib/common/libbesloudness/arsi_besloudness.c
INCLUDES += $(SOURCE_DIR)/middleware/lib/audio/hifi3/libdcrflt
C_FILES  += $(SOURCE_DIR)/middleware/lib/aurisys/lib/common/libdcrflt/arsi_dc_removal.c
LIBFLAGS += \
   -Wl,-L$(SOURCE_DIR)/middleware/lib/audio/hifi3/libbesloudness -lbesloudness -lm
LIBFLAGS += \
   -Wl,-L$(SOURCE_DIR)/middleware/lib/audio/hifi3/libdcrflt -ldcremoval

###################################################################
# 3rd Party Library
###################################################################
# default lib name for aurisys_set_buf/aurisys_get_buf
AURISYS_SET_GET_BUF_LIB_STR=\"aurisys_demo\"

ifeq ($(CFG_3RDPARTY_PROJECT),yes)

### NXP Speech (Call+VoIP)
ifeq ($(shell test -e $(THIRDPARTY_LIB_DIR)/aurisys/libnxp/libnxpspeech && echo -n yes),yes)
INCLUDES += $(THIRDPARTY_LIB_DIR)/aurisys/libnxp/libnxpspeech
C_FILES += $(THIRDPARTY_LIB_DIR)/aurisys/libnxp/libnxpspeech/aurisys_lvve_processing_lib.c
LIBFLAGS += -Wl,-L$(THIRDPARTY_LIB_DIR)/aurisys/libnxp/libnxpspeech/$(XT_TOOLS_VERSION) -lnxpspeech
CFLAGS += -DNXPSPEECH_DUMMY_LIB=0 -DRUN_ON_DEVICE
else
CFLAGS += -DNXPSPEECH_DUMMY_LIB=1
endif

### NXP Record
ifeq ($(shell test -e $(THIRDPARTY_LIB_DIR)/aurisys/libnxp/libnxprecord && echo -n yes),yes)
INCLUDES += $(THIRDPARTY_LIB_DIR)/aurisys/libnxp/libnxprecord
C_FILES += $(THIRDPARTY_LIB_DIR)/aurisys/libnxp/libnxprecord/aurisys_lvim_processing_lib.c
LIBFLAGS += -Wl,-L$(THIRDPARTY_LIB_DIR)/aurisys/libnxp/libnxprecord/$(XT_TOOLS_VERSION) -lnxprecord
CFLAGS += -DNXPRECORD_DUMMY_LIB=0 -DRUN_ON_DEVICE
else
CFLAGS += -DNXPRECORD_DUMMY_LIB=1
endif

### FV Speech (Call+VoIP+Record)
ifeq ($(shell test -e $(THIRDPARTY_LIB_DIR)/aurisys/libfv && echo -n yes),yes)
INCLUDES += $(THIRDPARTY_LIB_DIR)/aurisys/libfv
LIBFLAGS += -Wl,-L$(THIRDPARTY_LIB_DIR)/aurisys/libfv/$(XT_TOOLS_VERSION) -lFVSAM_P60Plus_v1007
CFLAGS += -DFV_DUMMY_LIB=0
else
CFLAGS += -DFV_DUMMY_LIB=1
endif

### mt6660
ifeq ($(shell test -e $(THIRDPARTY_LIB_DIR)/aurisys/mt6660 && echo -n yes),yes)
LIBFLAGS += -Wl,-L$(THIRDPARTY_LIB_DIR)/aurisys/mt6660 -lmt6660
C_FILES += $(THIRDPARTY_LIB_DIR)/aurisys/mt6660/mt6660_lib.c
CFLAGS += -DMT6660_SUPPORT=1
else
CFLAGS += -DMT6660_SUPPORT=0
endif

### NXP SmartPA
ifeq ($(shell test -e $(THIRDPARTY_LIB_DIR)/aurisys/libnxp/libnxpsmartpa && echo -n yes),yes)
INCLUDES += $(THIRDPARTY_LIB_DIR)/aurisys/libnxp/libnxpsmartpa
C_FILES += $(THIRDPARTY_LIB_DIR)/aurisys/libnxp/libnxpsmartpa/tfadsp_lib.c
LIBFLAGS += -Wl,-L$(THIRDPARTY_LIB_DIR)/aurisys/libnxp/libnxpsmartpa -lnxptfadsp
AURISYS_SET_GET_BUF_LIB_STR=\"smartpa_tfaxxxx\"
CFLAGS += -DNXP_SMARTPA_SUPPORT=1
else
CFLAGS += -DNXP_SMARTPA_SUPPORT=0
endif

###DIRAC
ifeq ($(shell test -e $(THIRDPARTY_LIB_DIR)/aurisys/dirac && echo -n yes),yes)
LIBFLAGS += \
   -Wl,-L$(THIRDPARTY_LIB_DIR)/aurisys/dirac -ldirac -lm
CFLAGS += -DDIRAC_SUPPORT=1
else
CFLAGS += -DDIRAC_SUPPORT=0
endif

else
CFLAGS += -DNXPSPEECH_DUMMY_LIB=1
CFLAGS += -DNXPRECORD_DUMMY_LIB=1
CFLAGS += -DFV_DUMMY_LIB=1
CFLAGS += -DMT6660_SUPPORT=0
CFLAGS += -DNXP_SMARTPA_SUPPORT=0
CFLAGS += -DDIRAC_SUPPORT=0
endif

CFLAGS += -DAURISYS_SET_GET_BUF_LIB_NAME=$(AURISYS_SET_GET_BUF_LIB_STR)

###################################################################
# Internal Library
###################################################################
ifeq ($(CFG_A2DP_OFFLOAD_SUPPORT),yes)
INCLUDES += $(SOURCE_DIR)/middleware/lib/audio/hifi3/sbcencoder
LIBFLAGS += \
   -Wl,-L$(SOURCE_DIR)/middleware/lib/audio/hifi3/sbcencoder -lsbcencoder
endif
endif


ifeq ($(CFG_MTK_AUDIO_FRAMEWORK_SUPPORT),yes)
INCLUDES += \
  $(SOURCE_DIR)/drivers/common/audio/tasks/primaydl \
  $(SOURCE_DIR)/drivers/common/audio/tasks/deepbufferdl \
  $(SOURCE_DIR)/drivers/common/audio/tasks/voipdl \
  $(SOURCE_DIR)/drivers/common/audio/tasks/audplayback \
  $(SOURCE_DIR)/drivers/common/audio/tasks/common \
  $(SOURCE_DIR)/drivers/common/audio/tasks/auddaemon \
  $(SOURCE_DIR)/drivers/common/audio/tasks/captureul1 \
  $(SOURCE_DIR)/drivers/common/audio/tasks/callfinal\
  $(SOURCE_DIR)/drivers/common/audio/tasks/ktv
C_FILES += \
  $(SOURCE_DIR)/drivers/common/audio/tasks/primaydl/audio_task_primary.c \
  $(SOURCE_DIR)/drivers/common/audio/tasks/deepbufferdl/audio_task_deepbuffer.c \
  $(SOURCE_DIR)/drivers/common/audio/tasks/voipdl/audio_task_voip.c \
  $(SOURCE_DIR)/drivers/common/audio/tasks/audplayback/audio_task_audplayback.c \
  $(SOURCE_DIR)/drivers/common/audio/tasks/common/audio_task_common.c \
  $(SOURCE_DIR)/drivers/common/audio/tasks/auddaemon/audio_task_daemon.c \
  $(SOURCE_DIR)/drivers/common/audio/tasks/captureul1/audio_task_capture_ul1.c \
  $(SOURCE_DIR)/drivers/common/audio/tasks/callfinal/audio_task_call_final.c\
  $(SOURCE_DIR)/drivers/common/audio/tasks/ktv/audio_task_ktv.c
endif

# add for offload playback
ifeq ($(CFG_MTK_AUDIO_TUNNELING_SUPPORT),yes)
INCLUDES += \
  $(SOURCE_DIR)/middleware/lib/audio/hifi3/mp3offload \
  $(SOURCE_DIR)/middleware/lib/audio/hifi3/blisrc \
  $(SOURCE_DIR)/drivers/common/audio/tasks/mp3 \
  $(DRIVERS_PLATFORM_DIR)/audio/tasks/mp3 \
  $(SOURCE_DIR)/middleware/lib/audio/hifi3/aacoffload
C_FILES += \
  $(SOURCE_DIR)/drivers/common/audio/tasks/mp3/audio_do_mp3.c \
  $(SOURCE_DIR)/drivers/common/audio/tasks/mp3/audio_task_offload_mp3.c \
  $(SOURCE_DIR)/drivers/common/audio/tasks/mp3/ADIF_Header.c \
  $(DRIVERS_PLATFORM_DIR)/audio/tasks/mp3/audio_dma_mp3.c
LIBFLAGS += \
  -Wl,-L$(SOURCE_DIR)/middleware/lib/audio/hifi3/mp3offload -lmp3dec_xtensa \
  -Wl,-L$(SOURCE_DIR)/middleware/lib/audio/hifi3/blisrc -lblisrc \
  -Wl,-L$(SOURCE_DIR)/middleware/lib/audio/hifi3/aacoffload -lheaacdec \
  -Wl,-L$(SOURCE_DIR)/middleware/lib/audio/hifi3/aacoffload -laacdec
endif

# add for a2dp offload
ifeq ($(CFG_A2DP_OFFLOAD_SUPPORT),yes)
INCLUDES += \
  $(SOURCE_DIR)/drivers/common/audio/tasks/a2dp \
  $(SOURCE_DIR)/drivers/common/audio/tasks/dataprovider
C_FILES += \
  $(SOURCE_DIR)/drivers/common/audio/tasks/a2dp/audio_task_a2dp.c \
  $(SOURCE_DIR)/drivers/common/audio/tasks/dataprovider/audio_task_dataprovider.c
endif
###################################################################
# Optional CompilerOption.mk used by project
###################################################################
-include $(PROJECT_DIR)/CompilerOption.mk
ifeq ($(CFG_TICKLESS_SUPPORT),yes)
CFLAGS += -DconfigUSE_TICKLESS_IDLE=1
CFLAGS += -DconfigUSE_WAKEUP_SOURCE=1
else
CFLAGS += -DconfigUSE_TICKLESS_IDLE=0
CFLAGS += -DconfigUSE_WAKEUP_SOURCE=0
endif

# 0:error, 1: warning, 2: info, 3: DEBUG, 4: verbose
#CFLAGS += -DFORCE_ALL_TASK_DEBUG_LEVEL=2
