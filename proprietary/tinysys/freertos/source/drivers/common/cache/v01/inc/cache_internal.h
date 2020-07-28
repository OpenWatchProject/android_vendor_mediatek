/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#include <stdint.h>

#ifndef __CACHE_INTERNAL_H__
#define __CACHE_INTERNAL_H__

#if 1
#ifdef __cplusplus
#define   __I     volatile             /*!< Defines 'read only' permissions                 */
#else
#define   __I     volatile const       /*!< Defines 'read only' permissions                 */
#endif
#define   __O     volatile             /*!< Defines 'write only' permissions                */
#define   __IO    volatile             /*!< Defines 'read / write' permissions              */
#ifndef NULL
#ifdef __cplusplus
#define NULL                (0) /*!< NULL */
#else
#define NULL                ((void *)(0)) /*!< NULL */
#endif
#endif
#endif
/* structure type to access the CACHE register
 */
typedef struct {
    __IO uint32_t CACHE_CON;
    __IO uint32_t CACHE_OP;
    __IO uint32_t CACHE_HCNT0L;
    __IO uint32_t CACHE_HCNT0U;
    __IO uint32_t CACHE_CCNT0L;
    __IO uint32_t CACHE_CCNT0U;
    __IO uint32_t CACHE_HCNT1L;
    __IO uint32_t CACHE_HCNT1U;
    __IO uint32_t CACHE_CCNT1L;
    __IO uint32_t CACHE_CCNT1U;
    uint32_t RESERVED0[1];
    __IO uint32_t CACHE_REGION_EN;
    uint32_t RESERVED1[2036];                  /**< (0x2000-12*4)/4 */
    __IO uint32_t CACHE_ENTRY_N[16];
    __IO uint32_t CACHE_END_ENTRY_N[16];
} CACHE_REGISTER_T;
/* CACHE_CON register definitions
 */
#define CACHE_CON_MCEN_OFFSET                   (0)
#define CACHE_CON_MCEN_MASK                     (1<<CACHE_CON_MCEN_OFFSET)
#define CACHE_CON_CNTEN0_OFFSET                 (2)
#define CACHE_CON_CNTEN0_MASK                   (1<<CACHE_CON_CNTEN0_OFFSET)
#define CACHE_CON_CNTEN1_OFFSET                 (3)
#define CACHE_CON_CNTEN1_MASK                   (1<<CACHE_CON_CNTEN1_OFFSET)
#define CACHE_CON_CACHESIZE_OFFSET              (8)
#define CACHE_CON_CACHESIZE_MASK                (3<<CACHE_CON_CACHESIZE_OFFSET)

#define CACHE_WAY_NUM                           (1<<10)


/* CACHE_OP register definitions
 */
#define CACHE_OP_EN_OFFSET                      (0)
#define CACHE_OP_EN_MASK                        (1<<CACHE_OP_EN_OFFSET)
#define CACHE_OP_OP_OFFSET                      (1)
#define CACHE_OP_OP_MASK                        (15<<CACHE_OP_OP_OFFSET)
#define CACHE_OP_TADDR_OFFSET                   (5)
#define CACHE_OP_TADDR_MASK                     (0x7FFFFFFUL<<CACHE_OP_TADDR_OFFSET)
/* CACHE_HCNT0L register definitions
 */
#define CACHE_HCNT0L_CHIT_CNT0_MASK             (0xFFFFFFFFUL<<0)
/* CACHE_HCNT0U register definitions
 */
#define CACHE_HCNT0U_CHIT_CNT0_MASK             (0xFFFFUL<<0)
/* CACHE_CCNT0L register definitions
 */
#define CACHE_CCNT0L_CACC_CNT0_MASK             (0xFFFFFFFFUL<<0)
/* CACHE_CCNT0U register definitions
 */
#define CACHE_CCNT0U_CACC_CNT0_MASK             (0xFFFFUL<<0)
/* CACHE_HCNT1L register definitions
 */
#define CACHE_HCNT1L_CHIT_CNT1_MASK             (0xFFFFFFFFUL<<0)
/* CACHE_HCNT1U register definitions
 */
#define CACHE_HCNT1U_CHIT_CNT1_MASK             (0xFFFFUL<<0)
/* CACHE_CCNT1L register definitions
 */
#define CACHE_CCNT1L_CACC_CNT1_MASK             (0xFFFFFFFFUL<<0)
/* CACHE_CCNT1U register definitions
 */
#define CACHE_CCNT1U_CACC_CNT1_MASK             (0xFFFFUL<<0)
/* CACHE_ENTRY_N register definitions
 */
#define CACHE_ENTRY_N_C_MASK                    (1<<8)
#define CACHE_ENTRY_BASEADDR_MASK               (0xFFFFFUL<<12)
/* CACHE_END_ENTRY_N register definitions
 */
#define CACHE_END_ENTRY_N_BASEADDR_MASK         (0xFFFFFUL<<12)
/* memory mapping of MT7687
 */
#define CMSYS_L1CACHE_BASE                      (0x01530000)
#define CACHE                                   ((CACHE_REGISTER_T *)CMSYS_L1CACHE_BASE)

typedef enum {
    HAL_CACHE_STATUS_INVALID_PARAMETER = -7,    /**< Invalid parameter */
    HAL_CACHE_STATUS_ERROR_BUSY = -6,           /**< CACHE is busy */
    HAL_CACHE_STATUS_ERROR_CACHE_SIZE = -5,     /**< CACHE size is invalid, total CACHE size is not a value of type #hal_cache_size_t*/
    HAL_CACHE_STATUS_ERROR_REGION = -4,         /**< CACHE region error, CACHE region is not a value of type #hal_cache_region_t */
    HAL_CACHE_STATUS_ERROR_REGION_ADDRESS = -3, /**< CACHE region address error, CACHE region address is not 4KB aligned */
    HAL_CACHE_STATUS_ERROR_REGION_SIZE = -2,    /**< CACHE region size error, CACHE region size is not a multiple of 4KB */
    HAL_CACHE_STATUS_ERROR = -1,                /**< CACHE error , imprecise error*/
    HAL_CACHE_STATUS_OK = 0                     /**< CACHE ok */
} hal_cache_status_t;
/** @brief CACHE size*/
typedef enum {
    HAL_CACHE_SIZE_0KB = 0,                     /**< No CACHE */
    HAL_CACHE_SIZE_8KB = 1,                     /**< CACHE size is 8KB */
    HAL_CACHE_SIZE_16KB = 2,                    /**< CACHE size is 16KB */
    HAL_CACHE_SIZE_32KB = 3,                    /**< CACHE size is 32KB */
    HAL_CACHE_SIZE_MAX                          /**< MAX CACHE size (invalid) */
} hal_cache_size_t;
/** @brief CACHE region number*/
typedef enum {
    HAL_CACHE_REGION_0 = 0,                     /**< CACHE region 0 */
    HAL_CACHE_REGION_1 = 1,                     /**< CACHE region 1 */
    HAL_CACHE_REGION_2 = 2,                     /**< CACHE region 2 */
    HAL_CACHE_REGION_3 = 3,                     /**< CACHE region 3 */
    HAL_CACHE_REGION_4 = 4,                     /**< CACHE region 4 */
    HAL_CACHE_REGION_5 = 5,                     /**< CACHE region 5 */
    HAL_CACHE_REGION_6 = 6,                     /**< CACHE region 6 */
    HAL_CACHE_REGION_7 = 7,                     /**< CACHE region 7 */
    HAL_CACHE_REGION_8 = 8,                     /**< CACHE region 8 */
    HAL_CACHE_REGION_9 = 9,                     /**< CACHE region 9 */
    HAL_CACHE_REGION_10 = 10,                   /**< CACHE region 10 */
    HAL_CACHE_REGION_11 = 11,                   /**< CACHE region 11 */
    HAL_CACHE_REGION_12 = 12,                   /**< CACHE region 12 */
    HAL_CACHE_REGION_13 = 13,                   /**< CACHE region 13 */
    HAL_CACHE_REGION_14 = 14,                   /**< CACHE region 14 */
    HAL_CACHE_REGION_15 = 15,                   /**< CACHE region 15 */
    HAL_CACHE_REGION_MAX                        /**< Max CACHE region number (invalid) */
} hal_cache_region_t;
/**
 * @}
 */
/*****************************************************************************
 * Structures
 *****************************************************************************/
/** @defgroup hal_cache_struct Struct
 *  @{
 */
/** @brief CACHE region config structure, that contains the start address of the region (must be 4KB aligned), the size of the region, and whether the region is cacheable or not.*/
typedef struct {
    uint32_t cache_region_address;              /**< CACHE region start address */
    uint32_t cache_region_size;                 /**< CACHE region size */
} hal_cache_region_config_t;

/*****************************************************************************
 * Functions
 *****************************************************************************/

/**
 * @brief   CACHE initialization function, set the default cacheable attribute for the project memory layout.
 * @return
 * #HAL_CACHE_STATUS_OK, CACHE initialization is successful. \n
 * #HAL_CACHE_STATUS_ERROR_BUSY, CACHE is busy.
 */
hal_cache_status_t hal_cache_init(uint32_t cache_type);


/**
 * @brief  CACHE deinitialization function. Make the CACHE module to its default state.
 * @return
 * #HAL_CACHE_STATUS_OK, CACHE deinitialization is successful.
 */
hal_cache_status_t hal_cache_deinit(uint32_t cache_type);


/**
 * @brief   CACHE enable function. Enable the CACHE settings during a memory access. @sa hal_cache_region_disable().
 * @return
 * #HAL_CACHE_STATUS_OK, CACHE enable is successful. \n
 * #HAL_CACHE_STATUS_ERROR_CACHE_SIZE, CACHE can not be enabled when CACHE size is 0KB.
 */
hal_cache_status_t hal_cache_enable(uint32_t cache_type);


/**
 * @brief CACHE disable function. Disable the CACHE settings. @sa hal_cache_region_enable().
 * @return
 * #HAL_CACHE_STATUS_OK, CACHE disable is successful.
 */
hal_cache_status_t hal_cache_disable(uint32_t cache_type);


/**
 * @brief CACHE region enable function. Once hal_cache_enable() is called, the settings of the specified region take effect. @sa hal_cache_enable().
 * @param[in] region is the enabled region, this parameter can only be a value of type #hal_cache_region_t.
 * @return
 * #HAL_CACHE_STATUS_OK, CACHE region enable is successful. \n
 * #HAL_CACHE_STATUS_ERROR_REGION, the region is invalid. \n
 * #HAL_CACHE_STATUS_ERROR, the region is not configured before enable.
 */
hal_cache_status_t hal_cache_region_enable(uint32_t cache_type, hal_cache_region_t region);


/**
 * @brief CACHE region disable function. When this function is called, the CACHE settings of corresponding region are disabled, even if the hal_cache_enable() function is called. @sa hal_cache_disable().
 * @param[in] region is the disabled region, this parameter is any value of type #hal_cache_region_t.
 * @return
 * #HAL_CACHE_STATUS_OK, CACHE region disable is successful. \n
 * #HAL_CACHE_STATUS_ERROR_REGION, the region is invalid.
 */
hal_cache_status_t hal_cache_region_disable(uint32_t cache_type, hal_cache_region_t region);


/**
 * @brief Set the total size of the CACHE.
 * @param[in] cache_size should only be any value of type #hal_cache_size_t.
 * @return
 * #HAL_CACHE_STATUS_OK, the CACHE size setting is successful. \n
 * #HAL_CACHE_STATUS_ERROR_CACHE_SIZE, the size of CACHE is invalid.
 */
hal_cache_status_t hal_cache_set_size(uint32_t cache_type, hal_cache_size_t cache_size);

/**
 * @brief   CACHE config function.
 * @param[in] region is the region that is configured.
 * @param[in] region_config is the configuration information of the region.
 * @return
 * #HAL_CACHE_STATUS_OK, CACHE region configuration is successful. \n
 * #HAL_CACHE_STATUS_INVALID_PARAMETER, region_config is NULL. \n
 * #HAL_CACHE_STATUS_ERROR_REGION, the region is invalid. \n
 * #HAL_CACHE_STATUS_ERROR_REGION_SIZE, the region size is invalid. \n
 * #HAL_CACHE_STATUS_ERROR_REGION_ADDRESS, the region address is invalid.
 */
hal_cache_status_t hal_cache_region_config(uint32_t cache_type, hal_cache_region_t region,
        const hal_cache_region_config_t *region_config);

/**
 * @brief  Invalidate one CACHE line by address.
 * @param[in] address is the start address of the memory that is invalidated.
 * @return
 * #HAL_CACHE_STATUS_OK, CACHE invalidate is successful. \n
 * #HAL_CACHE_STATUS_INVALID_PARAMETER, address is not CACHE line size aligned.
 */
hal_cache_status_t hal_cache_invalidate_one_cache_line(uint32_t cache_type, uint32_t address);

/**
 * @brief  Invalidate CACHE lines by address and length.
 * @param[in] address is the start address of the memory that is invalidated.
 * @param[in] length is the length of memory that to be invalidated.The unit of the memory length is in bytes and both address and length must be CACHE line size aligned when the API is called.
 * @return
 * #HAL_CACHE_STATUS_OK, CACHE invalidate is successful. \n
 * #HAL_CACHE_STATUS_INVALID_PARAMETER, either address or length is not CACHE line size aligned.
 */
hal_cache_status_t hal_cache_invalidate_multiple_cache_lines(uint32_t cache_type, uint32_t address, uint32_t length);

/**
 * @brief   Invalidate all CACHE lines.
 * @return
 * #HAL_CACHE_STATUS_OK, CACHE invalidate is successful.
 */
hal_cache_status_t hal_cache_invalidate_all_cache_lines(uint32_t cache_type);

/**
 * @brief   Invalidate all CACHE lines in ISR.
 * @return
 * #HAL_CACHE_STATUS_OK, CACHE invalidate is successful.
 */
hal_cache_status_t hal_cache_invalidate_all_cache_lines_from_isr(uint32_t cache_type);


/**
 * @brief  Flush one CACHE line by address.
 * @param[in] address is the start address of the memory that is flushed.
 * @return
 * #HAL_CACHE_STATUS_OK, CACHE flush is successful. \n
 * #HAL_CACHE_STATUS_INVALID_PARAMETER, address is not CACHE line size aligned.
 */
hal_cache_status_t hal_cache_flush_one_cache_line(uint32_t cache_type, uint32_t address);

/**
 * @brief  Flush CACHE lines by address and length.
 * @param[in] address is the start address of the memory that is flushed.
 * @param[in] length is the length of the memory that to be flushed. The unit of the memory length is in bytes and both address and length must be CACHE line size aligned when the API is called.
 * @return
 * #HAL_CACHE_STATUS_OK, CACHE flush is successful. \n
 * #HAL_CACHE_STATUS_INVALID_PARAMETER, either address or length is not CACHE line size aligned.
 */
hal_cache_status_t hal_cache_flush_multiple_cache_lines(uint32_t cache_type, uint32_t address, uint32_t length);


/**
 * @brief   Flush all CACHE lines.
 * @return
 * #HAL_CACHE_STATUS_OK, CACHE flush is successful.
 * @note Flush all CACHE lines.
 */
hal_cache_status_t hal_cache_flush_all_cache_lines(uint32_t cache_type);

/**
 * @brief   Flush all CACHE lines.
 * @return
 * #HAL_CACHE_STATUS_OK, CACHE flush is successful.
 * @note Flush all CACHE lines from ISR.
 */
hal_cache_status_t hal_cache_flush_all_cache_lines_from_isr(uint32_t cache_type);

void hal_cache_scp_sleep_protect_on();
void hal_cache_scp_sleep_protect_off();

uint32_t hal_cache_access_countl(uint32_t cache_type);
uint32_t hal_cache_hit_countl(uint32_t cache_type);
uint32_t hal_cache_access_countu(uint32_t cache_type);
uint32_t hal_cache_hit_countu(uint32_t cache_type);
uint64_t cache_hit_count(uint32_t cache_type);
uint64_t cache_access_count(uint32_t cache_type);

enum {
    BUS_QOS_NONE= 0,
    BUS_QOS_ULTRA = 1,
};

void bus_qos(uint32_t set);

typedef union {
    struct {
        uint32_t MCEN: 1;                       /**< CACHE enable */
        uint32_t MPEN: 1;                       /**< MPU enable */
        uint32_t CNTEN0: 1;                     /**< Hit counter0 enable */
        uint32_t CNTEN1: 1;                     /**< Hit counter1 enable */
        uint32_t MPDEFEN: 1;                    /**< MPU default protection */
        uint32_t _reserved0: 2;                 /**< Reserved */
        uint32_t MDRF: 1;                       /**< Early restart */
        uint32_t CACHESIZE: 2;                  /**< CACHE size */
        uint32_t _reserved1: 22;                /**< Reserved */
    } b;
    uint32_t w;
} CACHE_CON_Type;

typedef uint32_t CACHE_REGION_EN_Type;          /**< Region Enable Type */
typedef union {
    struct {
        uint32_t _reserved0: 5;                 /**< bit 0 */
        uint32_t ATTR: 3;                       /**< Attribute */
        uint32_t C: 1;                          /**< Cacheable bit */
        uint32_t _reserved1: 3;                 /**< Reserved */
        uint32_t BASEADDR: 20;                  /**< Base address of CACHE region */
    } b;
    uint32_t w;
} CACHE_ENTRY_N_Type;

typedef union {
    struct {
        uint32_t _reserved0: 12;                /**< Reserved */
        uint32_t BASEADDR: 20;                  /**< End address of CACHE region */
    } b;
    uint32_t w;
} CACHE_END_ENTRY_N_Type;

typedef struct {
    CACHE_ENTRY_N_Type cache_entry_n;
    CACHE_END_ENTRY_N_Type cache_end_entry_n;
} CACHE_ENTRY_Type;

extern CACHE_CON_Type g_cache_con;
extern CACHE_REGION_EN_Type g_cache_region_en;
extern CACHE_ENTRY_Type g_cache_entry[HAL_CACHE_REGION_MAX];
#define MTK_CACHE_LINE_SIZE         (32)        /**< CACHE line size is 8-word(32-byte) */
#define MTK_CACHE_REGION_SIZE_UNIT  (0x1000)    /**< CACHE region size must be 4KB aligned */
typedef enum {
    CACHE_INVALIDATE_ALL_LINES = 1,             /**< Invalidate all CACHE line */
    CACHE_INVALIDATE_ONE_LINE_BY_ADDRESS = 2,   /**< Invalidate one line by address */
    CACHE_INVALIDATE_ONE_LINE_BY_SET_WAY = 4,   /**< Invalidate one line by set and way */
    CACHE_FLUSH_ALL_LINES = 9,                  /**< Flush all CACHE lines */
    CACHE_FLUSH_ONE_LINE_BY_ADDRESS = 10,       /**< Flush one line by address */
    CACHE_FLUSH_ONE_LINE_BY_SET_WAY = 12        /**< Flush one line by set and way */
} cache_line_command_t;

#define CACHE_ICACHE 0
#define CACHE_DCACHE 1
#define CACHE_BASE            (0x405D4000)
#define CACHE_SEL(x) ((CACHE_REGISTER_T *)(CACHE_BASE+x*0x3000))


#define DRAM_REGION_BASE  0x80000 /* cache start from address 8M */
#define DRAM_REGION_LENGTH (0x10000000 - DRAM_REGION_BASE) /* cache end in the address, 256M */
#define MP3_DRAM_REGION_BASE  0x15000000
#define MP3_DRAM_REGION_LENGTH 0x400000
#ifdef CFG_CACHE_SUPPORT
#define SRAM_REGION_RO __attribute__ ((section (".sram_except_ro")))
#define SRAM_REGION_RW __attribute__ ((section (".sram_except_rw")))
#define SRAM_REGION_BSS __attribute__ ((section (".sram_except_bss")))
#define SRAM_REGION_VARIABLE __attribute__ ((section (".sram_except_variable")))
#define SRAM_REGION_FUNCTION __attribute__ ((section (".sram_except_func")))
#define DRAM_REGION_RO __attribute__ ((section (".dram_region_ro")))
#define DRAM_REGION_RW __attribute__ ((section (".dram_region_rw")))
#define DRAM_REGION_BSS __attribute__ ((section (".dram_region_bss")))
#define DRAM_REGION_VARIABLE __attribute__ ((section (".dram_region_variable")))
#define DRAM_REGION_FUNCTION __attribute__ ((section (".dram_region_func")))
#define L2TCM_CACHEABLE_REGION_VARIABLE __attribute__ ((section (".l2tcm_cacheable_region_variable")))
#define L2TCM_CACHEABLE_REGION_FUNCTION __attribute__ ((section (".l2tcm_cacheable_region_func")))
#else
#define SRAM_REGION_RO
#define SRAM_REGION_RW
#define SRAM_REGION_BSS
#define SRAM_REGION_VARIABLE
#define SRAM_REGION_FUNCTION
#define DRAM_REGION_RO
#define DRAM_REGION_RW
#define DRAM_REGION_BSS
#define DRAM_REGION_VARIABLE
#define DRAM_REGION_FUNCTION
#define L2TCM_CACHEABLE_REGION_VARIABLE
#define L2TCM_CACHEABLE_REGION_FUNCTION
#endif
#ifdef HAL_SLEEP_MANAGER_ENABLED
/* Save CACHE status before entering deepsleep */
void cache_status_save(void);
/* Restore CACHE status after leaving deepsleep */
void cache_status_restore(void);

#endif

/* symbol for the start address of cacheable memory*/
#define L1C_DRAM_ADDR       CFG_L1C_DRAM_ADDR
#define L1C_DRAM_SIZE       CFG_L1C_DRAM_SIZE
#define L1C_DRAM_ADDR_0     CFG_L1C_DRAM_ADDR_0
#define L1C_DRAM_SIZE_0     CFG_L1C_DRAM_SIZE_0
#define L1C_DRAM_ADDR_1     CFG_L1C_DRAM_ADDR_1
#define L1C_DRAM_SIZE_1     CFG_L1C_DRAM_SIZE_1

#endif
