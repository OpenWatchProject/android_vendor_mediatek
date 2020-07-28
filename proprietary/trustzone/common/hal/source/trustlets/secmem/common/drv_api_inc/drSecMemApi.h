/*
 * Copyright (c) 2014 - 2018 MediaTek Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __DR_SEC_MEM_API_H__
#define __DR_SEC_MEM_API_H__

#include <stdint.h>

#include "drSecMemInfo.h"

/* clang-format off */
#define _T_REE_   /* API that is used by REE secure memory kernel driver */
#define _T_SVP_   /* API that is used for SVP secure memory */
#define _T_PROT_  /* API that is used for protected share memory */
#define _T_2DFR_  /* API that is used for secure camera 2D_FR secure memory */
#define _T_WFD_   /* API that is used for WFD secure memory */
#define _T_SDSP_  /* API that is used for SDSP secure memory */
#define _T_MTEE_  /* API that is used for MTEE chunks secure memory */
#define _T_ALL_   /* API that is used for all type of secure memory */
#define _T_MISC_  /* API that is not related to any type of secure memory */
/* clang-format on */

enum MEM_TYPE {
    MEM_SVP = 0,
    MEM_SEC = MEM_SVP,
    MEM_PROT = 1,
    MEM_2D_FR = 2,
    MEM_WFD = 3,
    MEM_SDSP_SHARED = 4,
    MEM_SDSP_FIRMWARE = 5,
    MEM_HAPP_ELF = 6,
    MEM_HAPP_EXTRA = 7,

    MEM_TYPE_MAX
};

enum SECMEM_PL_RESERVED_MEM_TYPE {
    SECMEM_PL_RESERVED_M4U = 0,
    SECMEM_PL_RESERVED_CMDQ = 1,
    SECMEM_PL_RESERVED_SPI = 2,
    SECMEM_PL_RESERVED_I2C = 3,
};

/****************************************************************************
 *********** SVP Secure Memory APIs *****************************************
 ****************************************************************************/
_T_REE_ _T_SVP_ int drSecMemAlloc(uint64_t* handle, uint64_t size,
                                  uint64_t alignment);
_T_REE_ _T_SVP_ int drSecMemUnref(uint64_t handle, uint32_t* refcount);
_T_REE_ _T_SVP_ int drSecMemAllocZero(uint64_t* handle, uint64_t size,
                                      uint64_t alignment);
_T_REE_ _T_SVP_ int drSecMemEnable(uint64_t start_addr, uint64_t size);
_T_REE_ _T_SVP_ int drSecMemDisable(uint64_t* size);
_T_REE_ _T_SVP_ int drSecMemDumpInfo(void);

_T_SVP_ int drSecMemAllocPA(uint64_t* phy_addr, uint64_t size,
                            uint64_t alignment);
_T_SVP_ int drSecMemUnrefPA(uint64_t phy_addr, uint32_t* refcount);
_T_SVP_ int drSecMemQuery(void* mem_info);
_T_SVP_ int drSecMemQueryArray(void* mem_info, uint32_t len);
_T_SVP_ int drSecMemQueryPA(void* mem_info);
_T_SVP_ int drSecMemQueryArrayPA(void* mem_info, uint32_t len);
_T_SVP_ int drSecMemQueryPool(uint64_t* start_addr, uint64_t* size);

#define drSecMemFree(fmt, args...) drSecMemUnref(fmt, ##args)
#define drSecMemFreePA(fmt, args...) drSecMemUnrefPA(fmt, ##args)

/****************************************************************************
 *********** Protect-shared Memory APIs *************************************
 ****************************************************************************/
_T_REE_ _T_PROT_ int drProtMemConfig(uint64_t start_addr, uint64_t size);
_T_PROT_ int drProtMemRemappedPAQuery(uint64_t handle, uint64_t* pa);

/****************************************************************************
 *********** MTEE Multiple Chunk APIs ************************************
 ****************************************************************************/
_T_REE_ _T_MTEE_ int drMChunksMemRegionConfig(uint64_t start_addr,
                                              uint64_t size, uint32_t smem_type);

/****************************************************************************
 *********** WFD Secure Memory APIs *****************************************
 ****************************************************************************/
_T_REE_ _T_WFD_ int drWfdSmemAlloc(uint64_t* handle, uint64_t size,
                                   uint64_t alignment);
_T_REE_ _T_WFD_ int drWfdSmemUnref(uint64_t handle, uint32_t* refcount);
_T_REE_ _T_WFD_ int drWfdSmemAllocZero(uint64_t* handle, uint64_t size,
                                       uint64_t alignment);
_T_REE_ _T_WFD_ int drWfdSmemEnable(uint64_t start_addr, uint64_t size);
_T_REE_ _T_WFD_ int drWfdSmemDisable(uint64_t* size);
_T_REE_ _T_WFD_ int drWfdSmemDumpInfo(void);

/****************************************************************************
 *********** SDSP Secure Memory APIs *****************************************
 ****************************************************************************/
_T_REE_ _T_SDSP_ int drSdspSmemAlloc(uint64_t* handle, uint64_t size,
                                     uint64_t alignment);
_T_REE_ _T_SDSP_ int drSdspSmemUnref(uint64_t handle, uint32_t* refcount);
_T_REE_ _T_SDSP_ int drSdspSmemAllocZero(uint64_t* handle, uint64_t size,
                                         uint64_t alignment);
_T_REE_ _T_SDSP_ int drSdspSmemEnable(uint64_t start_addr, uint64_t size);
_T_REE_ _T_SDSP_ int drSdspSmemDisable(uint64_t* size);
_T_REE_ _T_SDSP_ int drSdspSmemDumpInfo(void);

/****************************************************************************
 *********** All Type of Secure Memory APIs *********************************
 ****************************************************************************/
_T_ALL_ int drMemPAQueryByType(uint64_t handle, uint64_t* pa, int mem_type);
_T_ALL_ int drMemPAQueryArrayByType(uint64_t* handle, uint64_t* pa,
                                    uint32_t len, int mem_type);
_T_ALL_ int drMemPoolQueryByType(uint64_t* start_addr, uint64_t* size,
                                 int mem_type);
_T_ALL_ int drRemappedMemPoolQueryByType(uint64_t* start_addr, uint64_t* size,
                                         int mem_type);

/****************************************************************************
 *********** MISC APIs ******************************************************
 ****************************************************************************/
_T_MISC_ int drSecMemQuerySharedMem(uint32_t* status);
_T_MISC_ int drSecMemQueryPlReservedMem(uint32_t pl_res_mem_type,
                                        uint32_t* addr, uint32_t* size);
_T_MISC_ int drSecmemDynamicDebugConfig(uint32_t enable);
_T_MISC_ int drSecmemForceHardwareProtection(void);
_T_MISC_ int drSecmemQueryGzTeeShmByName(const char* shm_name, uint64_t* pa,
                                         uint64_t* size);
_T_MISC_ int drSecmemQueryRemappedGzTeeShmByName(const char* shm_name,
                                                 uint64_t* pa, uint64_t* size);

#endif  // __DR_SEC_MEM_API_H__
