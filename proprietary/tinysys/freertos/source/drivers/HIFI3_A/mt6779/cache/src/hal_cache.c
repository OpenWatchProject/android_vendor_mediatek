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

#include "interrupt.h"
#include "hal_cache_internal.h"

hal_cache_status_t hal_cache_init (void)
{
    if(!is_in_isr())
        taskENTER_CRITICAL();
    {   /* Setup hardware pre-fetch */
        int prefetch = DEFAULT_CACHE_PREFETCH;
        /* int prefetch = xthal_get_cache_prefetch(); to check the prefetch setting */
        xthal_set_cache_prefetch(prefetch);
    }
    {   /* Setup cache ways */
        unsigned int icache_way = DEFAULT_ICACHE_WAY;
        unsigned int dcache_way = DEFAULT_DCACHE_WAY;
        /* unsigned int icache_way = xthal_icache_get_ways(); to check the I$ way setting */
        /* unsigned int dcache_way = xthal_dcache_get_ways(); to check the D$ way setting */
        xthal_icache_set_ways(icache_way);
        xthal_dcache_set_ways(dcache_way);
    }
    {   /* Flush & invalidate & unlock all cache lines */
        xthal_icache_all_unlock();          /* IIU invalidates the & line in addition to unlocking it in Eiger implementation */
        xthal_dcache_all_unlock();
        xthal_icache_all_invalidate();      /* Invalidating $ does not affect locked lines */
        xthal_dcache_all_writeback_inv();   /* Invalidating $ does not affect locked lines */
        /* Use "xthal_dcache_all_writeback();" if D$ no need for invalidation. */
        /* Use "xthal_dcache_all_invalidate();" if D$ no need for writeback, which is unlikely to happen. */
    }
    /* Set region attribute is done by MPU initialization. */
    /* Therefore, cache initialization can be started after MPU initialization is done. */
    if(!is_in_isr())
        taskEXIT_CRITICAL();
    return HAL_CACHE_STATUS_OK;
}

hal_cache_status_t hal_cache_deinit (void)
{
    if(!is_in_isr())
        taskENTER_CRITICAL();
    {   /* Flush & invalidate & unlock all cache lines */
        xthal_icache_all_unlock();          /* IIU invalidates the & line in addition to unlocking it in Eiger implementation */
        xthal_dcache_all_unlock();
        xthal_icache_all_invalidate();      /* Invalidating $ does not affect locked lines */
        xthal_dcache_all_writeback_inv();   /* Invalidating $ does not affect locked lines */
        /* Use "xthal_dcache_all_writeback();" if D$ no need for invalidation */
        /* Use "xthal_dcache_all_invalidate();" if D$ no need for writeback, which is unlikely to happen. */
    }
    if(!is_in_isr())
        taskEXIT_CRITICAL();
    return HAL_CACHE_STATUS_OK;
}

hal_cache_status_t hal_cache_region_operation (hal_cache_type_t type, hal_cache_operation_t operation, void *address, uint32_t size)
{
    hal_cache_status_t result = HAL_CACHE_STATUS_OK;
    UBaseType_t uxSavedInterruptStatus;
    uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
    {   /* Align the start & end address to cache line size. */
        uint32_t start_address = (uint32_t)address;
        uint32_t end_address = start_address + size;
        {   /* Align start address */
            uint32_t residual = start_address & CACHE_LINE_MASK;
            if (residual != 0) {
                size += residual;
                start_address -= residual;
            }
        }
        {   /* Align end address */
            uint32_t residual = end_address & CACHE_LINE_MASK;
            if (residual != 0) {
                residual = CACHE_LINE_SIZE - residual;
                size += residual;
                end_address += residual;
            }
        }
        address = (void *)start_address;
    }
    /* Check if address & size are both aligned to cache line size */
    if (((uint32_t)address & CACHE_LINE_MASK) == 0 && (size & CACHE_LINE_MASK) == 0) {
        if (type == HAL_CACHE_TYPE_INSTRUCTION && operation == HAL_CACHE_OPERATION_INVALIDATE) {
            xthal_icache_region_invalidate(address, (unsigned)size);
        } else if (type == HAL_CACHE_TYPE_INSTRUCTION && operation == HAL_CACHE_OPERATION_LOCK) {
            xthal_icache_region_lock(address, (unsigned)size);
        } else if (type == HAL_CACHE_TYPE_INSTRUCTION && operation == HAL_CACHE_OPERATION_UNLOCK) {
            xthal_icache_region_unlock(address, (unsigned)size);
        } else if (type == HAL_CACHE_TYPE_DATA && operation == HAL_CACHE_OPERATION_INVALIDATE) {
            xthal_dcache_region_invalidate(address, (unsigned)size);
        } else if (type == HAL_CACHE_TYPE_DATA && operation == HAL_CACHE_OPERATION_FLUSH) {
            xthal_dcache_region_writeback(address, (unsigned)size);
        } else if (type == HAL_CACHE_TYPE_DATA && operation == HAL_CACHE_OPERATION_FLUSH_INVALIDATE) {
            xthal_dcache_region_writeback_inv(address, (unsigned)size);
        } else if (type == HAL_CACHE_TYPE_DATA && operation == HAL_CACHE_OPERATION_LOCK) {
            xthal_dcache_region_lock(address, (unsigned)size);
        } else if (type == HAL_CACHE_TYPE_DATA && operation == HAL_CACHE_OPERATION_UNLOCK) {
            xthal_dcache_region_unlock(address, (unsigned)size);
        } else {
            result = HAL_CACHE_STATUS_ERROR;
        }
    } else {
        result = HAL_CACHE_STATUS_ERROR;
    }
    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
    return result;
}

hal_cache_status_t hal_cache_all_operation (hal_cache_type_t type, hal_cache_operation_t operation)
{
    hal_cache_status_t result = HAL_CACHE_STATUS_OK;
    UBaseType_t uxSavedInterruptStatus;
    uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
    if (type == HAL_CACHE_TYPE_INSTRUCTION && operation == HAL_CACHE_OPERATION_INVALIDATE) {
        xthal_icache_all_invalidate();
    } else if (type == HAL_CACHE_TYPE_INSTRUCTION && operation == HAL_CACHE_OPERATION_UNLOCK_INVALIDATE) {
        xthal_icache_all_unlock();
    } else if (type == HAL_CACHE_TYPE_DATA && operation == HAL_CACHE_OPERATION_INVALIDATE) {
        xthal_dcache_all_invalidate();
    } else if (type == HAL_CACHE_TYPE_DATA && operation == HAL_CACHE_OPERATION_FLUSH) {
        xthal_dcache_all_writeback();
    } else if (type == HAL_CACHE_TYPE_DATA && operation == HAL_CACHE_OPERATION_FLUSH_INVALIDATE) {
        xthal_dcache_all_writeback_inv();
    } else if (type == HAL_CACHE_TYPE_DATA && operation == HAL_CACHE_OPERATION_UNLOCK) {
        xthal_dcache_all_unlock();
    } else {
        result = HAL_CACHE_STATUS_ERROR;
    }
    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
    return result;
}

hal_cache_status_t hal_cache_set_prefetch_mode (hal_cache_type_t type, hal_cache_prefetch_mode_t mode)
{
    hal_cache_status_t result;
    if(!is_in_isr())
        taskENTER_CRITICAL();
    if (type == HAL_CACHE_TYPE_INSTRUCTION || type == HAL_CACHE_TYPE_DATA) {
        int32_t mask, shift, value;
        int32_t settings = (int32_t)xthal_get_cache_prefetch();
        if (type == HAL_CACHE_TYPE_INSTRUCTION) {
            mask  = INSTRUCTION_PREFETCH_MASK;
            shift = INSTRUCTION_PREFETCH_SHIFT;
        } else {
            mask  = DATA_PREFETCH_MASK;
            shift = DATA_PREFETCH_SHIFT;
        }
        value = prefetch_mode_to_value (mode);
        settings = (settings & ~mask) | ((value << shift) & mask);
        xthal_set_cache_prefetch(settings);
        result = HAL_CACHE_STATUS_OK;
    } else {    /* Error handling */
        result = HAL_CACHE_STATUS_ERROR;
    }
    if(!is_in_isr())
        taskEXIT_CRITICAL();
    return result;
}

hal_cache_prefetch_mode_t hal_cache_get_prefetch_mode (hal_cache_type_t type)
{
    hal_cache_prefetch_mode_t mode;
    if(!is_in_isr())
        taskENTER_CRITICAL();
    if (type == HAL_CACHE_TYPE_INSTRUCTION || type == HAL_CACHE_TYPE_DATA) {
        int32_t mask, shift, value;
        int32_t settings = (int32_t)xthal_get_cache_prefetch();
        if (type == HAL_CACHE_TYPE_INSTRUCTION) {
            mask  = INSTRUCTION_PREFETCH_MASK;
            shift = INSTRUCTION_PREFETCH_SHIFT;
        } else {
            mask  = DATA_PREFETCH_MASK;
            shift = DATA_PREFETCH_SHIFT;
        }
        value = (settings & mask) >> shift;
        mode = prefetch_value_to_mode (value);
    } else {    /* Error handling */
        mode = HAL_CACHE_PREFETCH_MODE_OFF;
    }
    if(!is_in_isr())
        taskEXIT_CRITICAL();
    return mode;
}

int32_t hal_cache_is_cacheable (void *address)
{
    int32_t result;
    if(!is_in_isr())
        taskENTER_CRITICAL();
    {
        xthal_MPU_entry entry = xthal_get_entry_for_address(address, NULL);
        unsigned int type = XTHAL_MPU_ENTRY_GET_MEMORY_TYPE(entry);
        result = xthal_is_cacheable(type);
    }
    if(!is_in_isr())
        taskEXIT_CRITICAL();
    return result;
}
