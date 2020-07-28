###################################################################
# Global feature options that need to align with Android ones
###################################################################
CFG_AUDIO_SUPPORT = no
CFG_MTK_AUDIO_TUNNELING_SUPPORT = no
CFG_MTK_AURISYS_PHONE_CALL_SUPPORT = no
CFG_MTK_VOW_SUPPORT = no
CFG_MTK_VOW_BARGE_IN_SUPPORT = no

###################################################################
# SCP internal feature options
###################################################################
CFG_TESTSUITE_SUPPORT = no
CFG_MODULE_INIT_SUPPORT = yes
CFG_XGPT_SUPPORT = yes
CFG_UART_SUPPORT = no
CFG_MTK_SCPUART_SUPPORT = yes
CFG_SCP_UART1_SUPPORT = no

# CFG_MTK_APUART_SUPPORT
# Do not use this with eng load or log may mix together and hard to recognzie
# Do not use this on lower power, it keeps infra always on
CFG_MTK_APUART_SUPPORT = no

##############################################################################
# When the option CFG_MTK_DYNAMIC_AP_UART_SWITCH is set to "yes", the code
# of UART will be built into the SCP image. This leads to a larger image.
# Set this option to "yes" only when you really know what you are doing.
# Otherwise, set it to "no".
##############################################################################
CFG_MTK_DYNAMIC_AP_UART_SWITCH = no
ifeq ($(CFG_MTK_DYNAMIC_AP_UART_SWITCH), yes)
CFG_UART_SUPPORT = yes
CFG_MTK_SCPUART_SUPPORT = no
CFG_SCP_UART1_SUPPORT = no
CFG_MTK_APUART_SUPPORT = yes
endif

CFG_SEM_SUPPORT = yes
CFG_IPC_SUPPORT = yes
CFG_ERR_INFO_SUPPORT = yes
CFG_LOGGER_SUPPORT = yes
CFG_LOGGER_TIMESTAMP_SUPPORT = yes
CFG_LOGGER_BOOTLOG_SUPPORT = yes
CFG_LOG_FILTER = yes
CFG_WDT_SUPPORT = yes
CFG_DMA_SUPPORT = yes
CFG_PMIC_WRAP_SUPPORT = yes
CFG_ETM_SUPPORT = no
CFG_I2C_SUPPORT = yes
CFG_SPI_SUPPORT = yes
CFG_CTP_SUPPORT = no
CFG_EINT_SUPPORT = yes
CFG_HEAP_GUARD_SUPPORT = no
CFG_VCORE_DVFS_SUPPORT = yes
CFG_MPU_DEBUG_SUPPORT = yes
CFG_RAMDUMP_SUPPORT = yes
CFG_DWT_SUPPORT = no
CFG_CHRE_SUPPORT = no
CFG_CHRE_256KB_SUPPORT = yes
CFG_SENSORS_INDRAM = no
CFG_MTK_CALIBRATION_V1_0 = yes
CFG_CONTEXTHUB_FW_SUPPORT = no
CFG_CORE_BENCHMARK_SUPPORT = no
CFG_SLT_SUPPORT = no
CFG_DVT_SUPPORT = no
CFG_CCCI_DVT_SUPPORT = no
CFG_CCCI_SUPPORT = yes
CFG_TICKLESS_SUPPORT = yes
CFG_FREERTOS_TRACE_SUPPORT = no
CFG_DEBUG_TASK_STATUS = yes
CFG_IRQ_MONITOR_SUPPORT = yes
CFG_IPI_STAMP_SUPPORT = no
CFG_OVERLAY_INIT_SUPPORT = no
CFG_OVERLAY_DEBUG_SUPPORT = no
CFG_HEAP_MONITOR_SUPPORT = yes

CFG_CM4_MODIFICATION = yes
CFG_I2C_CH0_DMA_SUPPORT = yes
CFG_I2C_CH1_DMA_SUPPORT = no

CFG_CNN_TO_SCP_BUF_SIZE = 0x0
CFG_SCP_TO_CNN_BUF_SIZE = 0x0
CFG_PBFR_SUPPORT = no
PBFR_SUPPORT_IOSTALL = no

###################################################################
# Platform Constant Parameter
###################################################################
CFG_SCP_SRAM_SIZE := 0x80000                 # 512 KB
CFG_L1C_DRAM_ADDR := 0x80000                 # For ALL DRAM Conf #
CFG_L1C_DRAM_SIZE := 0x100000
CFG_L1C_DRAM_ADDR_0 := 0x80000
CFG_L1C_DRAM_SIZE_0 := 18                     # MPU_SIZE_512KB = 18 #
CFG_L1C_DRAM_ADDR_1 := 0x100000
CFG_L1C_DRAM_SIZE_1 := 18                     # MPU_SIZE_512KB = 18 #
CFG_L1C_ICON := 0x100
CFG_L1C_DCON := 0x100
CFG_IRQ_DURATION_LIMIT := 97630
CFG_DBG_INFO := "$(PROJECT)"
CFG_TEXT_LIMIT_SIZE := 512K

###################################################################
# HW change
###################################################################
CFG_ONE_IPI_IRQ_DESIGN = yes

###################################################################
# Optional ProjectConfig.mk used by project
###################################################################
-include $(PROJECT_DIR)/ProjectConfig.mk

###################################################################
# Make the header of error info always exist to avoid build error.
###################################################################
INCLUDES += $(SOURCE_DIR)/drivers/common/err_info/

###################################################################
# Mandatory platform-specific resources
###################################################################
INCLUDES += \
  $(PLATFORM_DIR)/inc \
  $(SOURCE_DIR)/kernel/service/common/include \
  $(SOURCE_DIR)/kernel/CMSIS/Device/MTK/$(PLATFORM)/Include \
  $(SOURCE_DIR)/middleware/SensorHub \
  $(DRIVERS_PLATFORM_DIR)/feature_manager/inc

C_FILES += \
  $(PLATFORM_DIR)/src/main.c \
  $(PLATFORM_DIR)/src/platform.c \
  $(PLATFORM_DIR)/src/interrupt.c \
  $(SOURCE_DIR)/kernel/service/common/src/mtk_printf.c \
  $(SOURCE_DIR)/kernel/service/common/src/wakelock.c \
  $(PLATFORM_DIR)/src/scp_it.c \
  $(DRIVERS_PLATFORM_DIR)/feature_manager/src/feature_manager.c \
  $(PLATFORM_DIR)/src/scp_scpctl.c

# Add startup files to build
C_FILES += $(PLATFORM_DIR)/CMSIS/system.c
S_FILES += $(PLATFORM_DIR)/CMSIS/startup.S

ifeq ($(CFG_DRAMC_MONITOR_SUPPORT), yes)
# Add dramc (gating auto save) files to build, and please DO NOT remove.
INCLUDES += $(DRIVERS_PLATFORM_DIR)/dramc
C_FILES  += $(DRIVERS_PLATFORM_DIR)/dramc/dramc.c
endif

###############################################################################
# Delete "-Wno-error=format" when the check of arguments of printf() is needed.
###############################################################################
CFLAGS += -Wno-error=format

###################################################################
# Heap size config
###################################################################
# default heap size
$(eval TOTAL_HEAP_SIZE=$(shell echo $$(((40 * 1024)))))

ifeq ($(CFG_CCCI_SUPPORT),yes)
$(eval TOTAL_HEAP_SIZE=$(shell echo $$(($(TOTAL_HEAP_SIZE) + (4 * 1024)))))
endif

ifeq ($(CFG_MTK_VOW_SUPPORT),yes)
$(eval TOTAL_HEAP_SIZE=$(shell echo $$(($(TOTAL_HEAP_SIZE) + (8 * 1024)))))
endif

ifeq ($(CFG_MTK_SPEAKER_PROTECTION_SUPPORT),yes)
ifeq ($(CFG_SMARTPA_USE_CACHE),yes)
$(eval TOTAL_HEAP_SIZE=$(shell echo $$(($(TOTAL_HEAP_SIZE) + (12 * 1024)))))
else
$(eval TOTAL_HEAP_SIZE=$(shell echo $$(($(TOTAL_HEAP_SIZE) + (49 * 1024)))))
endif
endif

CFLAGS += -DconfigTOTAL_HEAP_SIZE=$(TOTAL_HEAP_SIZE)

ifeq ($(CFG_HEAP_MONITOR_SUPPORT),yes)
INCLUDES += $(SOURCE_DIR)/drivers/common/heap_monitor/inc/
C_FILES  += $(SOURCE_DIR)/drivers/common/heap_monitor/src/heap_monitor.c
endif
###################################################################
# Resources determined by configuration options
###################################################################
INCLUDES += -I$(SOURCE_DIR)/drivers/common/cache/v02/inc
ifeq ($(CFG_CACHE_SUPPORT), yes)
C_FILES  += $(SOURCE_DIR)/drivers/common/cache/v02/src/cache_internal.c
C_FILES  += $(SOURCE_DIR)/drivers/common/cache/v02/src/cache_dram_management.c
ifeq ($(CFG_DMGR_TEST), yes)
C_FILES  += $(SOURCE_DIR)/drivers/common/cache/test/cache_unit_test.c
endif
CAFLAG += -DCFG_CACHE_SUPPORT
CFG_CACHE_MON_DRAM_LATENCY = no
CFG_CACHE_MON_HIT_RATIO = yes
endif

ifeq ($(CFG_SLT_SUPPORT),yes)
INCLUDES += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_slt/inc
C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_slt/src/slt.c
C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_sample/src/i2c_testsuite.c
endif

ifeq ($(CFG_TEST_PROGRAM_SUPPORT),yes)
    CFLAGS += -DSEMU_TESTCASE
    INCLUDES += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/semu_test/inc/ts_slt/inc
    C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/semu_test/src/semu_test.c
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
ifeq (yes,$(CFG_PBFR_SUPPORT))
C_FILES  +=  $(TINYSYS_SECURE_DIR)/middleware/pbfr/pbfr_cm4.c
INCLUDES +=  $(TINYSYS_SECURE_DIR)/middleware/pbfr/inc
ifeq (yes, $(PBFR_SUPPORT_IOSTALL))
CFLAGS += -DPBFR_SUPPORT_IOSTALL
endif
endif

ifeq ($(CFG_TESTSUITE_SUPPORT),yes)
INCLUDES += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/inc
INCLUDES += $(SOURCE_DIR)/middleware/lib/console/include
C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_sample/src/EINT_testsuite.c
C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_sample/src/plat_testsuite.c
C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_sample/src/sample.c
C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_platform/src/platform.c
ifeq ($(CFG_VCORE_DVFS_SUPPORT),yes)
C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_dvfs/src/dvfs_test.c
endif
ifeq ($(CFG_CCCI_DVT_SUPPORT),yes)
INCLUDES += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/inc
INCLUDES += $(SOURCE_DIR)/middleware/lib/console/include
INCLUDES += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_ccif/inc
C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_ccif/src/ccif_hal_implement.c
C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_ccif/src/ccif_slt_helper.c
C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_ccif/src/com_ccif_dvt_case_master.c
C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_ccif/src/com_ccif_util_lib.c
C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_ccif/src/plat_hw_info.c
C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_ccif/src/ts_ccif.c
endif
ifeq ($(CFG_I2C_SUPPORT),yes)
C_FILES  += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/src/ts_sample/src/i2c_testsuite.c
endif
C_FILES  += middleware/lib/console/console.c
endif

ifeq ($(CFG_CORE_BENCHMARK_SUPPORT),yes)
C_FILES  += $(TINYSYS_SECURE_DIR)/middleware/benchmark/coremark_v1.0/core_list_join.c
C_FILES  += $(TINYSYS_SECURE_DIR)/middleware/benchmark/coremark_v1.0/core_main.c
C_FILES  += $(TINYSYS_SECURE_DIR)/middleware/benchmark/coremark_v1.0/core_matrix.c
C_FILES  += $(TINYSYS_SECURE_DIR)/middleware/benchmark/coremark_v1.0/core_state.c
C_FILES  += $(TINYSYS_SECURE_DIR)/middleware/benchmark/coremark_v1.0/core_util.c
C_FILES  += $(TINYSYS_SECURE_DIR)/middleware/benchmark/coremark_v1.0/core_portme.c
CFLAGS   += -DPERFORMANCE_RUN=1 -DITERATIONS=500 -DBUILD_WITH_FREERTOS
endif

ifeq ($(CFG_MODULE_INIT_SUPPORT),yes)
C_FILES  += $(SOURCE_DIR)/kernel/service/common/src/module_init.c
endif

ifeq ($(CFG_OVERLAY_INIT_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/overlay/inc
C_FILES  += $(DRIVERS_PLATFORM_DIR)/overlay/src/mtk_overlay_init.c
endif

ifeq ($(CFG_XGPT_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/xgpt/inc/
C_FILES  += $(DRIVERS_PLATFORM_DIR)/xgpt/src/xgpt.c
C_FILES  += $(SOURCE_DIR)/kernel/service/common/src/utils.c
endif

ifeq ($(CFG_PMIC_WRAP_SUPPORT),yes)
INCLUDES += $(SOURCE_DIR)/drivers/common/pmic_wrap/v2/inc \
  $(DRIVERS_PLATFORM_DIR)/pmic_wrap/inc
C_FILES  += \
   $(SOURCE_DIR)/drivers/common/pmic_wrap/v2/pmic_wrap.c
endif

ifeq ($(CFG_UART_SUPPORT),yes)
C_FILES  += $(SOURCE_DIR)/drivers/common/uart/v01/uart.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/uart/uart_platform.c
endif

ifeq ($(CFG_SEM_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/sem/inc
C_FILES  += $(SOURCE_DIR)/drivers/common/sem/v02/src/scp_sem.c
endif

ifeq ($(CFG_IPC_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/ipi/inc
C_FILES  += $(SOURCE_DIR)/drivers/common/ipi/v02/src/scp_ipi.c
ifeq ($(CFG_ERR_INFO_SUPPORT),yes)
C_FILES  += $(SOURCE_DIR)/drivers/common/err_info/scp_err_info.c
endif
endif

ifeq ($(CFG_LOGGER_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/logger/inc
C_FILES  += $(SOURCE_DIR)/drivers/common/logger/v01/src/scp_logger.c
endif

ifeq ($(CFG_WDT_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/wdt/inc
C_FILES  += $(DRIVERS_PLATFORM_DIR)/wdt/src/wdt.c
endif

ifeq ($(CFG_DMA_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/dma
C_FILES  += $(SOURCE_DIR)/drivers/common/dma/v03/src/dma.c
C_FILES  += $(SOURCE_DIR)/drivers/common/dma/v03/src/dma_api.c
endif

ifeq ($(CFG_ETM_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/etm
C_FILES  += $(DRIVERS_PLATFORM_DIR)/etm/etm.c
endif

#ifeq ($(CFG_CTP_SUPPORT),yes)
#endif

ifeq ($(CFG_EINT_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/eint/inc
C_FILES  += $(SOURCE_DIR)/drivers/common/eint/v02/src/eint.c
endif

ifeq ($(CFG_HEAP_GUARD_SUPPORT),yes)
C_FILES  += $(RTOS_SRC_DIR)/portable/MemMang/mtk_HeapGuard.c
LDFLAGS += -Wl, -wrap=pvPortMalloc -Wl, -wrap=vPortFree
endif

ifeq ($(CFG_VCORE_DVFS_SUPPORT),yes)
INCLUDES += $(SOURCE_DIR)/kernel/service/common/include/
INCLUDES += $(DRIVERS_PLATFORM_DIR)/dvfs/inc
INCLUDES += $(SOURCE_DIR)/drivers/common/dvfs/v01/inc
INCLUDES += $(TINYSYS_SECURE_DIR)/$(PROJECT_DIR)/testsuite/inc/
C_FILES  += $(SOURCE_DIR)/drivers/common/dvfs/v01/src/dvfs_common.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/dvfs/src/dvfs.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/dvfs/src/sleep.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/dvfs/src/resource_req.c
endif

ifeq ($(CFG_SENSORHUB_SUPPORT), yes)
INCLUDES += $(HAL_PLATFORM_DIR)/driver/ipi/inc
INCLUDES += $(SOURCE_DIR)/middleware/lib/console/include
INCLUDES += $(SOURCE_DIR)/middleware/SensorHub
INCLUDES += $(SOURCE_DIR)/kernel/FreeRTOS/Source/include
INCLUDES += $(PLATFORM_DIR)/inc
INCLUDES += $(SOURCE_DIR)/kernel/FreeRTOS/Source/portable/GCC/ARM_CM4F
INCLUDES += $(SOURCE_DIR)/kernel/service/common/include
C_FILES  += $(SOURCE_DIR)/middleware/SensorHub/sensor_manager.c
C_FILES  += $(SOURCE_DIR)/middleware/SensorHub/sensor_manager_fw.c
endif

ifeq ($(CFG_SENSORHUB_TEST_SUPPORT), yes)
C_FILES  += $(SOURCE_DIR)/middleware/SensorHub/sensorframeworktest.c
C_FILES  += $(SOURCE_DIR)/middleware/SensorHub/FakeAccelSensorDriver.c
endif

ifeq ($(CFG_FLP_SUPPORT), yes)
CFG_GEOFENCE_SUPPORT = yes
endif
ifeq ($(CFG_GEOFENCE_SUPPORT), yes)
CFG_CCCI_SUPPORT = yes
endif
ifeq ($(CFG_CCCI_SUPPORT), yes)
INCLUDES += -I$(DRIVERS_PLATFORM_DIR)/ccci
INCLUDES += -I$(SOURCE_DIR)/drivers/common/ccci
C_FILES  += $(SOURCE_DIR)/drivers/common/ccci/ccci.c
C_FILES  += $(SOURCE_DIR)/drivers/common/ccci/ccism_ringbuf.c
C_FILES  += $(SOURCE_DIR)/drivers/common/ccci/sensor_modem.c
C_FILES  += $(SOURCE_DIR)/drivers/common/ccci/ccci_hw_ccif.c
endif

ifeq ($(CFG_MPU_DEBUG_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/mpu/inc
C_FILES  += $(SOURCE_DIR)/drivers/common/mpu/v01/src/mpu_mtk.c
endif

ifeq ($(CFG_MTK_VOW_SUPPORT),yes)
CFG_AUDIO_SUPPORT = yes
CFG_MTK_VOW_SEAMLESS_SUPPORT = yes
endif

# add for spk_protection
ifeq ($(CFG_MTK_SPEAKER_PROTECTION_SUPPORT),yes)
CFG_AUDIO_SUPPORT = yes
CFG_MTK_AUDIO_CM4_DMA_SUPPORT = yes
endif

ifeq ($(CFG_SENSOR_GPIO_SWITCH_SUPPORT),yes)
INCLUDES += $(TINYSYS_SECURE_DIR)/middleware/sensor/gpio_switch/mt6779/inc
C_FILES  += $(TINYSYS_SECURE_DIR)/middleware/sensor/gpio_switch/mt6779/gpio_i2c_switch.c
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
C_FILES  += $(SOURCE_DIR)/drivers/common/audio/framework/audio_messenger_ipi.c
C_FILES  += $(SOURCE_DIR)/drivers/common/audio/framework/audio_task_factory.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/audio/hardware/audio_hw.c
C_FILES  += $(DRIVERS_PLATFORM_DIR)/audio/hardware/audio_irq.c
C_FILES  += $(SOURCE_DIR)/drivers/common/audio/framework/audio_task_utility.c

ifeq ($(CFG_MTK_AUDIO_CM4_DMA_SUPPORT),yes)
INCLUDES += $(SOURCE_DIR)/drivers/common/audio/tasks/controller
C_FILES  += $(SOURCE_DIR)/drivers/common/audio/tasks/controller/audio_task_controller.c
C_FILES  += $(SOURCE_DIR)/drivers/common/audio/framework/audio_ipi_dma.c
endif
endif

ifeq ($(CFG_CHRE_SUPPORT), yes)
INCLUDES += -I$(DRIVERS_PLATFORM_DIR)/i2c/inc/contexthub
INCLUDES += -I$(SOURCE_DIR)/../../../hardware/contexthub/firmware/inc
include $(FEATURE_CONFIG_DIR)/chre.mk
endif

ifeq ($(CFG_DWT_SUPPORT),yes)
INCLUDES += $(DRIVERS_PLATFORM_DIR)/dwt/inc
C_FILES  += $(DRIVERS_PLATFORM_DIR)/dwt/src/dwt.c
endif

# add for speaker_protection
ifeq ($(CFG_MTK_SPEAKER_PROTECTION_SUPPORT),yes)
INCLUDES += \
  $(SOURCE_DIR)/drivers/common/audio/tasks/spkprotect \
  $(SOURCE_DIR)/middleware/lib/audio_utility \
  $(SOURCE_DIR)/middleware/lib/aurisys/lib/cm4/dummylib
C_FILES += \
  $(SOURCE_DIR)/drivers/common/audio/tasks/spkprotect/audio_task_speaker_protection.c
#MT6660
LIBFLAGS += \
  -Wl,-L$(SOURCE_DIR)/middleware/lib/aurisys/lib/cm4/mtk_smartpa,-lMT6660_LIB
ifeq ($(CFG_MTK_SMARTPA_BYPASS_LIB),yes)
  CFLAGS += -DMTK_SMARTPA_BYPASS_LIB
endif
endif

include $(CLEAR_FEATURE_VARS)
FEATURE_CONFIG := CFG_MTK_VOW_SUPPORT
FEATURE_NAME   := FEATURE_MTK_VOW
FEATURE_INCLUDES := \
  $(SOURCE_DIR)/middleware/lib/audio/cm4/vow/inc \
  $(SOURCE_DIR)/middleware/lib/audio/cm4/vow/inc/mt6779 \
  $(DRIVERS_PLATFORM_DIR)/audio/hardware \
  $(SOURCE_DIR)/drivers/common/audio/tasks/vow/v03 \
  $(SOURCE_DIR)/drivers/common/audio/hardware/vow/v02 \
  $(DRIVERS_PLATFORM_DIR)/audio/tasks/vow
FEATURE_C_FILES := \
  $(SOURCE_DIR)/drivers/common/audio/tasks/vow/v03/audio_do_vow.c \
  $(SOURCE_DIR)/drivers/common/audio/tasks/vow/v03/audio_task_vow.c \
  $(DRIVERS_PLATFORM_DIR)/audio/hardware/vow_hw.c \
  $(DRIVERS_PLATFORM_DIR)/audio/tasks/vow/audio_dma_vow.c \
  $(SOURCE_DIR)/middleware/lib/audio/cm4/vow/swVAD.c
FEATURE_LIBFLAGS := \
  -L$(SOURCE_DIR)/middleware/lib/audio/cm4/vow -lvow -lm
include $(REGISTER_FEATURE)

include $(CLEAR_FEATURE_VARS)
FEATURE_CONFIG := CFG_MTK_VOW_BARGE_IN_SUPPORT
FEATURE_NAME   := FEATURE_MTK_VOW_BARGE_IN
FEATURE_LIBFLAGS := \
  -L$(SOURCE_DIR)/middleware/lib/audio/cm4/vow -laec \
  -L$(SOURCE_DIR)/middleware/lib/audio/cm4/vow/mt6779 -lblisrc_accurate_vow
include $(REGISTER_FEATURE)

include $(CLEAR_FEATURE_VARS)
FEATURE_CONFIG := CFG_FEATURE01_SUPPORT
FEATURE_NAME   := FEATURE01
FEATURE_INCLUDES := \
  $(SOURCE_DIR)/middleware/feature01 \
  $(SOURCE_DIR)/middleware/feature01_task
FEATURE_C_FILES := \
  $(SOURCE_DIR)/middleware/feature01/feature01.c
FEATURE_SCP_C_FILES := \
  $(SOURCE_DIR)/middleware/feature01_task/feature01_sample.c
include $(REGISTER_FEATURE)

include $(CLEAR_FEATURE_VARS)
FEATURE_CONFIG := CFG_FEATURE02_SUPPORT
FEATURE_NAME   := FEATURE02
FEATURE_INCLUDES := \
  $(SOURCE_DIR)/middleware/feature02
FEATURE_C_FILES := \
  $(SOURCE_DIR)/middleware/feature02/feature02.c
include $(REGISTER_FEATURE)

include $(CLEAR_FEATURE_VARS)
FEATURE_CONFIG := CFG_FEATURE03_SUPPORT
FEATURE_NAME   := FEATURE03
FEATURE_INCLUDES := \
  $(SOURCE_DIR)/middleware/feature03
FEATURE_C_FILES := \
  $(SOURCE_DIR)/middleware/feature03/feature03.c
include $(REGISTER_FEATURE)



###################################################################
# Optional Dynamic Object definition file that includes all
# supported Dynamic Object sets
###################################################################
-include $(PLATFORM_DIR)/dos.mk

###################################################################
# Optional CompilerOption.mk used by project
###################################################################
-include $(PROJECT_DIR)/CompilerOption.mk
ifeq ($(CFG_TICKLESS_SUPPORT),yes)
CFLAGS += -DconfigUSE_TICKLESS_IDLE=1
else
CFLAGS += -DconfigUSE_TICKLESS_IDLE=0
endif

###################################################################
# Post processing
###################################################################
CFG_DO_ENABLED :=
ifneq (,$(strip $($(PROCESSOR).ALL_DOS)))
CFG_DO_ENABLED = yes
INCLUDES += $(DO_COMMON_INCLUDES)
C_FILES += $(DO_SCP_C_FILES)
S_FILES += $(DO_SCP_S_FILES)
endif

# define platform name in upper case
CFLAGS += -D$(shell echo $(PLATFORM) | tr a-z A-Z)

