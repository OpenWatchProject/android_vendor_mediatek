/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2017
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#ifndef __HAL_CACHE_H__
#define __HAL_CACHE_H__

#include <stdint.h>

typedef enum {
    HAL_CACHE_STATUS_OK    =  0,
    HAL_CACHE_STATUS_ERROR = -1
} hal_cache_status_t;

typedef enum {
    HAL_CACHE_TYPE_INSTRUCTION = 1,
    HAL_CACHE_TYPE_DATA        = 2
} hal_cache_type_t;

typedef enum {
    HAL_CACHE_OPERATION_INVALIDATE        = 1,
    HAL_CACHE_OPERATION_FLUSH             = 2,
    HAL_CACHE_OPERATION_FLUSH_INVALIDATE  = 3,
    HAL_CACHE_OPERATION_LOCK              = 4,
    HAL_CACHE_OPERATION_UNLOCK            = 5,
    HAL_CACHE_OPERATION_UNLOCK_INVALIDATE = 6
} hal_cache_operation_t;

typedef enum {
    HAL_CACHE_PREFETCH_MODE_OFF              = 0,
    HAL_CACHE_PREFETCH_MODE_LOW_USE_TABLE    = 1,
    HAL_CACHE_PREFETCH_MODE_LOW_ANY_MISS     = 3,
    HAL_CACHE_PREFETCH_MODE_MEDIUM_USE_TABLE = 4,
    HAL_CACHE_PREFETCH_MODE_MEDIUM_NO_TABLE  = 5,
    HAL_CACHE_PREFETCH_MODE_HIGH             = 8
} hal_cache_prefetch_mode_t;

/********************************************************************************************\
|   Function:                                                                                |
|       hal_cache_init                                                                       |
|                                                                                            |
|   Description:                                                                             |
|       Initialize instruction cache & data cache ways, setup hardware prefetch, flush &     |
|       invalidate & unlock all cache lines.                                                 |
|                                                                                            |
|   Return Values:                                                                           |
|       HAL_CACHE_STATUS_OK    --> normal                                                    |
|       HAL_CACHE_STATUS_ERROR --> error                                                     |
|                                                                                            |
\********************************************************************************************/
hal_cache_status_t hal_cache_init (void);

/********************************************************************************************\
|   Function:                                                                                |
|       hal_cache_deinit                                                                     |
|                                                                                            |
|   Description:                                                                             |
|       Flush & invalidate & unlock all cache lines.                                         |
|                                                                                            |
|   Return Values:                                                                           |
|       HAL_CACHE_STATUS_OK    --> normal                                                    |
|       HAL_CACHE_STATUS_ERROR --> error                                                     |
|                                                                                            |
\********************************************************************************************/
hal_cache_status_t hal_cache_deinit (void);

/********************************************************************************************\
|   Function:                                                                                |
|       hal_cache_region_operation                                                           |
|                                                                                            |
|   Description:                                                                             |
|       Operate instruction cache or data cache by region with given start address & size.   |
|                                                                                            |
|   Parameters:                                                                              |
|       type                                                                                 |
|           Indicate the cache type for the given operation.                                 |
|           HAL_CACHE_TYPE_INSTRUCTION --> instruction cache                                 |
|           HAL_CACHE_TYPE_DATA        --> data cache                                        |
|       operation                                                                            |
|           Indicate the cache operation for the given region.                               |
|           HAL_CACHE_OPERATION_INVALIDATE       --> invalidate region                       |
|           HAL_CACHE_OPERATION_FLUSH            --> flush region                            |
|           HAL_CACHE_OPERATION_FLUSH_INVALIDATE --> flush & invalidate region               |
|           HAL_CACHE_OPERATION_LOCK             --> lock region                             |
|           HAL_CACHE_OPERATION_UNLOCK           --> unlock region                           |
|       address                                                                              |
|           Start address of the given region.                                               |
|       size                                                                                 |
|           Size of the given region. (Unit: byte)                                           |
|                                                                                            |
|   Restriction:                                                                             |
|       The type / operation combinations are limited.                                       |
|       An invalid combination will cause the API to return error.                           |
|       The valid combinations are listed below:                                             |
|       type                       operation                                                 |
|       HAL_CACHE_TYPE_INSTRUCTION HAL_CACHE_OPERATION_INVALIDATE                            |
|       HAL_CACHE_TYPE_INSTRUCTION HAL_CACHE_OPERATION_LOCK                                  |
|       HAL_CACHE_TYPE_INSTRUCTION HAL_CACHE_OPERATION_UNLOCK                                |
|       HAL_CACHE_TYPE_DATA        HAL_CACHE_OPERATION_INVALIDATE                            |
|       HAL_CACHE_TYPE_DATA        HAL_CACHE_OPERATION_FLUSH                                 |
|       HAL_CACHE_TYPE_DATA        HAL_CACHE_OPERATION_FLUSH_INVALIDATE                      |
|       HAL_CACHE_TYPE_DATA        HAL_CACHE_OPERATION_LOCK                                  |
|       HAL_CACHE_TYPE_DATA        HAL_CACHE_OPERATION_UNLOCK                                |
|                                                                                            |
|   Return Values:                                                                           |
|       HAL_CACHE_STATUS_OK    --> normal                                                    |
|       HAL_CACHE_STATUS_ERROR --> error                                                     |
|                                                                                            |
\********************************************************************************************/
hal_cache_status_t hal_cache_region_operation (hal_cache_type_t type, hal_cache_operation_t operation, void *address, uint32_t size);

/********************************************************************************************\
|   Function:                                                                                |
|       hal_cache_all_operation                                                              |
|                                                                                            |
|   Description:                                                                             |
|       Operate all instruction cache lines or data cache lines.                             |
|                                                                                            |
|   Parameters:                                                                              |
|       type                                                                                 |
|           Indicate the cache type for the given operation.                                 |
|           HAL_CACHE_TYPE_INSTRUCTION --> instruction cache                                 |
|           HAL_CACHE_TYPE_DATA        --> data cache                                        |
|       operation                                                                            |
|           Indicate the cache operation for the given region.                               |
|           HAL_CACHE_OPERATION_INVALIDATE        --> invalidate entire cache                |
|           HAL_CACHE_OPERATION_FLUSH             --> flush entire cache                     |
|           HAL_CACHE_OPERATION_FLUSH_INVALIDATE  --> flush & invalidate entire cache        |
|           HAL_CACHE_OPERATION_UNLOCK            --> unlock entire cache                    |
|           HAL_CACHE_OPERATION_UNLOCK_INVALIDATE --> unlock & invalidate entire cache       |
|                                                                                            |
|   Restriction:                                                                             |
|       The type / operation combinations are limited.                                       |
|       An invalid combination will cause the API to return error.                           |
|       The valid combinations are listed below:                                             |
|       type                       operation                                                 |
|       HAL_CACHE_TYPE_INSTRUCTION HAL_CACHE_OPERATION_INVALIDATE                            |
|       HAL_CACHE_TYPE_INSTRUCTION HAL_CACHE_OPERATION_UNLOCK_INVALIDATE                     |
|       HAL_CACHE_TYPE_DATA        HAL_CACHE_OPERATION_INVALIDATE                            |
|       HAL_CACHE_TYPE_DATA        HAL_CACHE_OPERATION_FLUSH                                 |
|       HAL_CACHE_TYPE_DATA        HAL_CACHE_OPERATION_FLUSH_INVALIDATE                      |
|       HAL_CACHE_TYPE_DATA        HAL_CACHE_OPERATION_UNLOCK                                |
|                                                                                            |
|   Return Values:                                                                           |
|       HAL_CACHE_STATUS_OK    --> normal                                                    |
|       HAL_CACHE_STATUS_ERROR --> error                                                     |
|                                                                                            |
\********************************************************************************************/
hal_cache_status_t hal_cache_all_operation (hal_cache_type_t type, hal_cache_operation_t operation);

/********************************************************************************************\
|   Function:                                                                                |
|       hal_cache_set_prefetch_mode                                                          |
|                                                                                            |
|   Description:                                                                             |
|       Set hardware prefetch mode for the cache prefetch strategy aggressiveness.           |
|                                                                                            |
|   Parameters:                                                                              |
|       type                                                                                 |
|           Indicate the cache type for the given mode setting.                              |
|           HAL_CACHE_TYPE_INSTRUCTION --> instruction cache                                 |
|           HAL_CACHE_TYPE_DATA        --> data cache                                        |
|       mode                                                                                 |
|           Indicate the hardware prefetch mode.                                             |
|           HAL_CACHE_PREFETCH_MODE_OFF                                                      |
|               no prefetch                                                                  |
|           HAL_CACHE_PREFETCH_MODE_LOW_USE_TABLE                                            |
|               low aggressiveness with miss address table used                              |
|               (Prefetch 1 cache line ahead when a cache miss hits miss address table or    |
|               prefetch entry.)                                                             |
|           HAL_CACHE_PREFETCH_MODE_LOW_ANY_MISS                                             |
|               low aggressiveness for any cache miss                                        |
|               (Prefetch 1 cache line ahead on any cache miss.)                             |
|           HAL_CACHE_PREFETCH_MODE_MEDIUM_USE_TABLE                                         |
|               medium aggressiveness with miss address table used                           |
|               (Prefetch 2 cache lines ahead when a cache miss hits miss address table or   |
|               prefetch entry.)                                                             |
|           HAL_CACHE_PREFETCH_MODE_MEDIUM_NO_TABLE                                          |
|               medium aggressiveness without miss address table                             |
|               (Prefetch 2 cache lines ahead when a cache miss hits in prefetch entry.)     |
|               (Prefetch 1 cache line ahead on any cache miss does not hit in prefetch      |
|               entry.)                                                                      |
|           HAL_CACHE_PREFETCH_MODE_HIGH                                                     |
|               high aggressiveness                                                          |
|               (Prefetch 4 cache lines ahead when a cache miss hits in prefetch entry.)     |
|               (Prefetch 2 cache lines ahead on any cache miss does not hit in prefetch     |
|               entry.)                                                                      |
|                                                                                            |
|   Return Values:                                                                           |
|       HAL_CACHE_STATUS_OK    --> normal                                                    |
|       HAL_CACHE_STATUS_ERROR --> error                                                     |
|                                                                                            |
\********************************************************************************************/
hal_cache_status_t hal_cache_set_prefetch_mode (hal_cache_type_t type, hal_cache_prefetch_mode_t mode);

/********************************************************************************************\
|   Function:                                                                                |
|       hal_cache_get_prefetch_mode                                                          |
|                                                                                            |
|   Description:                                                                             |
|       Get hardware prefetch mode for the cache prefetch strategy aggressiveness.           |
|                                                                                            |
|   Parameter:                                                                               |
|       type                                                                                 |
|           Indicate the cache type.                                                         |
|           HAL_CACHE_TYPE_INSTRUCTION --> instruction cache                                 |
|           HAL_CACHE_TYPE_DATA        --> data cache                                        |
|                                                                                            |
|   Return Values:                                                                           |
|       HAL_CACHE_PREFETCH_MODE_OFF                                                          |
|           no prefetch                                                                      |
|       HAL_CACHE_PREFETCH_MODE_LOW_USE_TABLE                                                |
|           low aggressiveness with miss address table used                                  |
|       HAL_CACHE_PREFETCH_MODE_LOW_ANY_MISS                                                 |
|           low aggressiveness for any cache miss                                            |
|       HAL_CACHE_PREFETCH_MODE_MEDIUM_USE_TABLE                                             |
|           medium aggressiveness with miss address table used                               |
|       HAL_CACHE_PREFETCH_MODE_MEDIUM_NO_TABLE                                              |
|           medium aggressiveness without miss address table                                 |
|       HAL_CACHE_PREFETCH_MODE_HIGH                                                         |
|           high aggressiveness                                                              |
|                                                                                            |
\********************************************************************************************/
hal_cache_prefetch_mode_t hal_cache_get_prefetch_mode (hal_cache_type_t type);

/********************************************************************************************\
|   Function:                                                                                |
|       hal_cache_is_cacheable                                                               |
|                                                                                            |
|   Description:                                                                             |
|       Check whether the given address is cacheable or not.                                 |
|                                                                                            |
|   Parameter:                                                                               |
|       address --> address of the given instruction or data                                 |
|                                                                                            |
|   Return Values:                                                                           |
|       0       The given address is not cacheable.                                          |
|       Non-0   The given address is cacheable.                                              |
|                                                                                            |
\********************************************************************************************/
int32_t hal_cache_is_cacheable (void *address);

/********************************************************************************************\
|   Macro:                                                                                   |
|       hal_cache_prefetch_instruction                                                       |
|                                                                                            |
|   Description:                                                                             |
|       Apply software prefetch for the given instruction address.                           |
|                                                                                            |
|   Parameter:                                                                               |
|       address --> address of the given instruction                                         |
|                                                                                            |
\********************************************************************************************/
#define hal_cache_prefetch_instruction(address) do {                \
    void *target = (void *)(address);                               \
    __asm__ __volatile__("ipf %0, 0" :: "a"(target) : "memory");    \
} while(0)

/********************************************************************************************\
|   Macro:                                                                                   |
|       hal_cache_prefetch_lock_instruction                                                  |
|                                                                                            |
|   Description:                                                                             |
|       Apply software prefetch & lock cache line for the given instruction address.         |
|                                                                                            |
|   Parameter:                                                                               |
|       address --> address of the given instruction                                         |
|                                                                                            |
\********************************************************************************************/
#define hal_cache_prefetch_lock_instruction(address) do {           \
    void *target = (void *)(address);                               \
    __asm__ __volatile__("ipfl %0, 0" :: "a"(target) : "memory");   \
} while(0)

/********************************************************************************************\
|   Macro:                                                                                   |
|       hal_cache_unlock_instruction                                                         |
|                                                                                            |
|   Description:                                                                             |
|       Unlock cache line for the given instruction address.                                 |
|                                                                                            |
|   Parameter:                                                                               |
|       address --> address of the given instruction                                         |
|                                                                                            |
\********************************************************************************************/
#define hal_cache_unlock_instruction(address) do {                  \
    void *target = (void *)(address);                               \
    __asm__ __volatile__("ihu %0, 0" :: "a"(target) : "memory");    \
} while(0)

/********************************************************************************************\
|   Macro:                                                                                   |
|       hal_cache_prefetch_data                                                              |
|                                                                                            |
|   Description:                                                                             |
|       Apply software prefetch for the given data address.                                  |
|                                                                                            |
|   Parameter:                                                                               |
|       address --> address of the given data                                                |
|                                                                                            |
\********************************************************************************************/
#define hal_cache_prefetch_data(address) do {                       \
    void *target = (void *)(address);                               \
    __asm__ __volatile__("dpfw %0, 0" :: "a"(target) : "memory");   \
} while(0)

/********************************************************************************************\
|   Macro:                                                                                   |
|       hal_cache_prefetch_lock_data                                                         |
|                                                                                            |
|   Description:                                                                             |
|       Apply software prefetch & lock cache line for the given data address.                |
|                                                                                            |
|   Parameter:                                                                               |
|       address --> address of the given data                                                |
|                                                                                            |
\********************************************************************************************/
#define hal_cache_prefetch_lock_data(address) do {                  \
    void *target = (void *)(address);                               \
    __asm__ __volatile__("dpfl %0, 0" :: "a"(target) : "memory");   \
} while(0)

/********************************************************************************************\
|   Macro:                                                                                   |
|       hal_cache_unlock_data                                                                |
|                                                                                            |
|   Description:                                                                             |
|       Unlock cache line for the given data address.                                        |
|                                                                                            |
|   Parameter:                                                                               |
|       address --> address of the given data                                                |
|                                                                                            |
\********************************************************************************************/
#define hal_cache_unlock_data(address) do {                         \
    void *target = (void *)(address);                               \
    __asm__ __volatile__("dhu %0, 0" :: "a"(target) : "memory");    \
} while(0)

#endif /* __HAL_CACHE_H__ */
