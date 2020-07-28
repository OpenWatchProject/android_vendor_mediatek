
/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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
/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Header Files
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
#include <audio_type.h>
#include <audio_ringbuf.h>
#include <dma.h>
#include <dvfs.h>
#include <audio_task_offload_mp3_params.h>
#include <hal_cache.h>

/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Static Variables
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Static Functions
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/

static int mp3_dma_length_fourbyte(uint32_t len, uint32_t dst_addr,
                                   uint32_t src_addr)
{
    if (len % 4 || dst_addr % 4 || src_addr % 4) {
        return 0;  // config can't four byte
    }
    else {
        return 1;
    }
}
static void mp3_memcpy_dma(const char *target, const char *source, size_t count,
                         uint8_t Dramplaystate)
{
    if (count == 0 || target == source) {  /* nothing to do */
        return;
    }
    if (Dramplaystate == 0) {  // sram
        mp3_dma_transaction_wrap((uint32_t)target, (uint32_t)source, count, false);
    } else {  // dram
        mp3_dma_transaction_wrap((uint32_t)target, (uint32_t)source, count, true);
    }
}
/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        DMA WRAP API
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/


/**
* copy count number bytes from buf to RingBuf1
* @param RingBuf1 ring buffer copy from
* @param buf copy to
* @param count number of bytes need to copy
*/

void mp3_RingBuf_copyFromLinear_dma(struct RingBuf *RingBuf1, const char *buf,
                                uint32_t count, const char bDramPlayback)
{
    int spaceIHave;
    char *end = RingBuf1->pBufBase + RingBuf1->bufLen;
    // count buffer data I have
    spaceIHave = RingBuf1->bufLen - RingBuf_getDataCount(RingBuf1) - 8;

    //PRINTF_E("base: %p, len: %d, spaceIHave %d ,count %d\n",RingBuf1->pBufBase, RingBuf1->bufLen, spaceIHave,count);

    // if not enough, printf error
    if(spaceIHave < count)
        PRINTF_E("spaceIHave %d < count %d\n",spaceIHave,count);

    if (RingBuf1->pRead <= RingBuf1->pWrite) {
        int w2e = end - RingBuf1->pWrite;
        if (count <= w2e) {
            mp3_memcpy_dma(RingBuf1->pWrite, buf, count, bDramPlayback);
            RingBuf1->pWrite += count;
            if (RingBuf1->pWrite == end) {
                RingBuf1->pWrite = RingBuf1->pBufBase;
            }
        } else {
            mp3_memcpy_dma(RingBuf1->pWrite, buf, w2e, bDramPlayback);
            mp3_memcpy_dma(RingBuf1->pBufBase, buf + w2e, count - w2e, bDramPlayback);
            RingBuf1->pWrite = RingBuf1->pBufBase + count - w2e;
        }
    } else {
        mp3_memcpy_dma(RingBuf1->pWrite, buf, count, bDramPlayback);
        RingBuf1->pWrite += count;
    }
   // PRINTF_E("pRead: %p, pWrite: %p\n",RingBuf1->pRead, RingBuf1->pWrite);

}

int mp3_dma_transaction_wrap(uint32_t dst_addr, uint32_t src_addr,
                             uint32_t len, uint8_t IsDram)
{
    DMA_RESULT ret = 0;
    if (mp3_dma_length_fourbyte(len, dst_addr, src_addr) == 0) {
        uint32_t need_add  = 4 - (dst_addr % 4);
        // ret = scp_dma_transaction(dst_addr + need_add, src_addr, len, MP3_DMA_ID, NO_RESERVED);
        memcpy((void *)(dst_addr + need_add), (void *)src_addr, len);


        unsigned int pa_addr = (unsigned int)dst_addr + need_add;
        hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                                   HAL_CACHE_OPERATION_FLUSH_INVALIDATE,
                                   (void *)pa_addr,
                                   len
                                  );

        memmove((void *)dst_addr, (void *)(dst_addr + need_add), len);

        PRINTF_E("frog ret = %d, 0x%x, 0x%x, 0x%x\n", ret, dst_addr, src_addr, len);

    }
    else {
        hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                                   HAL_CACHE_OPERATION_INVALIDATE,
                                   (void *)src_addr,
                                   len
                                  );
        memcpy((void *)dst_addr, (void *)src_addr, len);
        PRINTF_E("dma2 ret = %d, 0x%x, 0x%x, 0x%x\n", ret, dst_addr, src_addr, len);
        hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                                   HAL_CACHE_OPERATION_FLUSH_INVALIDATE,
                                   (void *)dst_addr,
                                   len
                                   );
    }

    return ret;
}
