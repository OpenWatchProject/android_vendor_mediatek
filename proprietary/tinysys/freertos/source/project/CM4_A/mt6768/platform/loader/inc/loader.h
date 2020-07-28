#ifndef _LOADER_H_
#define _LOADER_H_

#define CFGREG_BASE     0x405C0000

#define MPU_PROTECT     0
#define WDT_SUPPORT     0
#define TCM_FORCE_LOCK  0
#define TCM_LOCK_SUPPORT  0

#define LOADER_BASE     0x00000000
#define LOADER_SIZE     0x9     /* 1KB */
#define MPU_BASE        0xE000ED80
#define MPU_SIZE        0x5     /* 64 bytes */
#define SRAM_BASE       0x00000000
#define SRAM_SIZE       0x12    /* 512KB */
#define MEM_BASE        0x20000000
#define MEM_SPACE       0x1F    /* 4GB */
#define DRAM_REMAP_ADR  0x20000000    /*SCP EXT ADDR 0*/
#define DRAM_REMAP_MASK 0x00080000
#define UNUSED_REGION   0x1C    /* 512MB (0x0000_0000~0x2000_0000) */

#define REMAP_CFG1      (CFGREG_BASE + 0x120)
#define REMAP_CFG2      (CFGREG_BASE + 0x124)
#define REMAP_CFG3      (CFGREG_BASE + 0x128)

#define REMAP_VAL1      0x07060504      /*0x21987654*/
#define REMAP_VAL2      0x02010008      /*0x112200A3*/
#define REMAP_VAL3      0x01000A03

#define TCM_LOCK_CFG    (CFGREG_BASE + 0x10)      /*SCP A  LOCK count*/
#define TCM_LOCK_SHIFT  8
#define TCM_LOCK_UNIT   12      /* 4KB */
#define SCP_ONE_TIME_LOCK    (CFGREG_BASE + 0xDC)      /*SCP A  LOCK count*/
#define SCP_SECURE_CRTL      (CFGREG_BASE + 0xE0)      /*SCP A  LOCK count*/
#define TCM_EN_BIT      (1 << 20)

#define WDT_REG_BASE    (CFGREG_BASE + 0x84)       /*SCP A WDT*/
#define WDT_EN          (0 << 31)
#define WDT_INTV        (10 * 32768)
#define WDT_INTV_MASK   0x000FFFFF
#define WDT_TICK        0x00000001
#define MPU_TYPE        0xE000ED90
#define MPU_CTRL        0xE000ED94
#define MPU_RBAR        0xE000ED9C
#define MPU_RASR        0xE000EDA0
#define VTOR            0xE000ED08
#define CPACR           0xE000ED88
#define CPACR_VAL       (0xF << 20)

/* struct scp_region_info_st */
#define OFF_LOADER_START       0x0
#define OFF_LOADER_SIZE        0x4
#define OFF_FW_START           0x8
#define OFF_FW_SIZE            0xc
#define OFF_CACHE_START        0x10
#define OFF_CACHE_SIZE         0x14
#define OFF_CACHE_BACKUP_START 0x18
#define OFF_STRUCT_SIZE        0x1c
#define OFF_LOG_THRU_AP_UART   0x20
#define OFF_TASK_CONTEXT_PT    0x24
#define OFF_CFG_L1C_ICON       0x28
#define OFF_CFG_L1C_DCON       0x2c
#define OFF_SCPCTL             0x30
#define OFF_PARAM_START        0x34

#endif
