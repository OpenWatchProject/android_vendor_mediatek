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
#include <audio_type.h>
#include <audio_ringbuf.h>
#include "audio_task_factory.h"
#include <dma.h>
#include "audio_log.h"
#include "audio_errno.h"
#include "audio_dsp_hw_hal.h"
#include "audio_hw.h"
#include "audio_hw_reg.h"
#include <audio_ringbuf.h>
#include <audio_task_utility.h>

#define AFE_HW_ALIGNMENT (0xFFFFFF80)

//#define DEBUG_VERBOSE

unsigned int table_sgen_golden_values[128] = { // sgen test table
    0x000FE511, 0x000FE511, 0x001C4470, 0x001C4470,
    0x00285E1E, 0x00285E1E, 0x00341447, 0x00341447,
    0x003F4A0A, 0x003F4A0A, 0x0049E3C4, 0x0049E3C4,
    0x0053C752, 0x0053C752, 0x005CDC55, 0x005CDC55,
    0x00650C66, 0x00650C66, 0x006C4357, 0x006C4357,
    0x00726F5E, 0x00726F5E, 0x00778143, 0x00778143,
    0x007B6C87, 0x007B6C87, 0x007E2781, 0x007E2781,
    0x007FAB74, 0x007FAB74, 0x007FF4A6, 0x007FF4A6,
    0x007F0260, 0x007F0260, 0x007CD6F8, 0x007CD6F8,
    0x007977C9, 0x007977C9, 0x0074ED21, 0x0074ED21,
    0x006F4234, 0x006F4234, 0x006884F9, 0x006884F9,
    0x0060C610, 0x0060C610, 0x00581890, 0x00581890,
    0x004E91DF, 0x004E91DF, 0x00444979, 0x00444979,
    0x003958B7, 0x003958B7, 0x002DDA93, 0x002DDA93,
    0x0021EB63, 0x0021EB63, 0x0015A893, 0x0015A893,
    0x0009305C, 0x0009305C, 0x00FCA17F, 0x00FCA17F,
    0x00F01AEF, 0x00F01AEF, 0x00E3BB90, 0x00E3BB90,
    0x00D7A1E2, 0x00D7A1E2, 0x00CBEBB9, 0x00CBEBB9,
    0x00C0B5F6, 0x00C0B5F6, 0x00B61C3C, 0x00B61C3C,
    0x00AC38AE, 0x00AC38AE, 0x00A323AB, 0x00A323AB,
    0x009AF39A, 0x009AF39A, 0x0093BCA9, 0x0093BCA9,
    0x008D90A2, 0x008D90A2, 0x00887EBD, 0x00887EBD,
    0x00849379, 0x00849379, 0x0081D87F, 0x0081D87F,
    0x0080548C, 0x0080548C, 0x00800B5A, 0x00800B5A,
    0x0080FDA0, 0x0080FDA0, 0x00832908, 0x00832908,
    0x00868837, 0x00868837, 0x008B12DF, 0x008B12DF,
    0x0090BDCC, 0x0090BDCC, 0x00977B07, 0x00977B07,
    0x009F39F0, 0x009F39F0, 0x00A7E770, 0x00A7E770,
    0x00B16E21, 0x00B16E21, 0x00BBB687, 0x00BBB687,
    0x00C6A749, 0x00C6A749, 0x00D2256D, 0x00D2256D,
    0x00DE149D, 0x00DE149D, 0x00EA576D, 0x00EA576D,
    0x00F6CFA4, 0x00F6CFA4, 0x00035E81, 0x00035E81
};


struct task_hw_buffer_info {
    int id;                     /* key */
    AudioTask *user;
    struct audio_hw_buffer hw_buf;
    struct RingBuf ring_buf;
    int state;
    char *prev_idx;
    int task_scene_id;
} __attribute__((aligned(128)));

static struct task_hw_buffer_info
        temp_hw_buffer_info[MT6779_MEMIF_NUM]; // TODO remove magic number


static void dump_mem_reg(int memif)
{
    uint32_t dl_base, dl_end, dl_cur;
    switch (memif) {
        case MT6779_MEMIF_DL1:
            dl_base = Afe_Get_Reg(AFE_DL1_BASE);
            dl_end = Afe_Get_Reg(AFE_DL1_END);
            dl_cur = Afe_Get_Reg(AFE_DL1_CUR);
            break;
        case MT6779_MEMIF_DL2:
            dl_base = Afe_Get_Reg(AFE_DL2_BASE);
            dl_end = Afe_Get_Reg(AFE_DL2_END);
            dl_cur = Afe_Get_Reg(AFE_DL2_CUR);
            break;
        case MT6779_MEMIF_DL3:
            dl_base = Afe_Get_Reg(AFE_DL3_BASE);
            dl_end = Afe_Get_Reg(AFE_DL3_END);
            dl_cur = Afe_Get_Reg(AFE_DL3_CUR);
            break;
        case MT6779_MEMIF_DL4:
            dl_base = Afe_Get_Reg(AFE_DL4_BASE);
            dl_end = Afe_Get_Reg(AFE_DL4_END);
            dl_cur = Afe_Get_Reg(AFE_DL4_CUR);
            break;
        case MT6779_MEMIF_DL5:
            dl_base = Afe_Get_Reg(AFE_DL5_BASE);
            dl_end = Afe_Get_Reg(AFE_DL5_END);
            dl_cur = Afe_Get_Reg(AFE_DL5_CUR);
            break;
        case MT6779_MEMIF_DL6:
            dl_base = Afe_Get_Reg(AFE_DL6_BASE);
            dl_end = Afe_Get_Reg(AFE_DL6_END);
            dl_cur = Afe_Get_Reg(AFE_DL6_CUR);
            break;
        case MT6779_MEMIF_DL7:
            dl_base = Afe_Get_Reg(AFE_DL7_BASE);
            dl_end = Afe_Get_Reg(AFE_DL7_END);
            dl_cur = Afe_Get_Reg(AFE_DL7_CUR);
            break;
        case MT6779_MEMIF_DL8:
            dl_base = Afe_Get_Reg(AFE_DL8_BASE);
            dl_end = Afe_Get_Reg(AFE_DL8_END);
            dl_cur = Afe_Get_Reg(AFE_DL8_CUR);
            break;
        case MT6779_MEMIF_VUL12:
            dl_base = Afe_Get_Reg(AFE_VUL12_BASE);
            dl_end = Afe_Get_Reg(AFE_VUL12_END);
            dl_cur = Afe_Get_Reg(AFE_VUL12_CUR);
            break;
        case MT6779_MEMIF_VUL2:
            dl_base = Afe_Get_Reg(AFE_VUL2_BASE);
            dl_end = Afe_Get_Reg(AFE_VUL2_END);
            dl_cur = Afe_Get_Reg(AFE_VUL2_CUR);
            break;
        case MT6779_MEMIF_VUL3:
            dl_base = Afe_Get_Reg(AFE_VUL3_BASE);
            dl_end = Afe_Get_Reg(AFE_VUL3_END);
            dl_cur = Afe_Get_Reg(AFE_VUL3_CUR);
            break;
        case MT6779_MEMIF_VUL4:
            dl_base = Afe_Get_Reg(AFE_VUL4_BASE);
            dl_end = Afe_Get_Reg(AFE_VUL4_END);
            dl_cur = Afe_Get_Reg(AFE_VUL4_CUR);
            break;
        case MT6779_MEMIF_VUL5:
            dl_base = Afe_Get_Reg(AFE_VUL5_BASE);
            dl_end = Afe_Get_Reg(AFE_VUL5_END);
            dl_cur = Afe_Get_Reg(AFE_VUL5_CUR);
            break;
        case MT6779_MEMIF_VUL6:
            dl_base = Afe_Get_Reg(AFE_VUL6_BASE);
            dl_end = Afe_Get_Reg(AFE_VUL6_END);
            dl_cur = Afe_Get_Reg(AFE_VUL6_CUR);
            break;
        case MT6779_MEMIF_AWB:
            dl_base = Afe_Get_Reg(AFE_AWB_BASE);
            dl_end = Afe_Get_Reg(AFE_AWB_END);
            dl_cur = Afe_Get_Reg(AFE_AWB_CUR);
            break;
        case MT6779_MEMIF_AWB2:
            dl_base = Afe_Get_Reg(AFE_AWB2_BASE);
            dl_end = Afe_Get_Reg(AFE_AWB2_END);
            dl_cur = Afe_Get_Reg(AFE_AWB2_CUR);
            break;
        default:
            AUD_LOG_W("%s() memif[%x]\n", __func__, memif);
            break;
    }

    AUD_LOG_D("%s() dl_base[%x] dl_end[%x] dl_cur[%x]\n",
              __func__, dl_base, dl_end, dl_cur);

}


static void dump_irq_reg(int irqline)
{
    uint32_t irq_mcu_sta, irq_scp_en , irq_mcu_en , afe_dac_con0 , irq_dsp_en, irq_cnt = 0;
    irq_mcu_sta = Afe_Get_Reg(AFE_IRQ_MCU_STATUS);
    irq_dsp_en = Afe_Get_Reg(AFE_IRQ_MCU_DSP_EN);
    irq_scp_en = Afe_Get_Reg(AFE_IRQ_MCU_SCP_EN);
    irq_mcu_en = Afe_Get_Reg(AFE_IRQ_MCU_EN);
    afe_dac_con0 = Afe_Get_Reg(AFE_DAC_CON0);

    switch (irqline) {
        case 0:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT0);
            break;
        case 1:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT1);
            break;
        case 2:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT2);
            break;
        case 3:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT3);
            break;
        case 4:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT4);
            break;
        case 5:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT5);
            break;
        case 6:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT6);
            break;
        case 7:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT7);
            break;
        case 8:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT8);
            break;
        case 9:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT9);
            break;
        case 10:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT10);
            break;
        case 11:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT11);
            break;
        case 12:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT12);
            break;
        case 13:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT13);
            break;
        case 14:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT14);
            break;
        case 15:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT15);
            break;
        case 16:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT16);
            break;
        case 17:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT17);
            break;
        case 18:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT18);
            break;
        case 19:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT19);
            break;
        case 20:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT20);
            break;
        case 21:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT21);
            break;
        case 24:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT24);
            break;
        case 25:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT25);
            break;
        case 26:
            irq_cnt = Afe_Get_Reg(AFE_IRQ_MCU_CNT26);
            break;
        default:
            AUD_LOG_W("%s() irq_cnt[%x]\n", __func__, irq_cnt);
            break;
    }

    AUD_LOG_D("%s() irq_mcu_sta[%x] irq_dsp_en[%x] scp_en[%x] mcu_en[%x] afe_dac_con0[%x] irq_cnt[%x]\n",
              __func__,
              irq_mcu_sta, irq_scp_en, irq_scp_en, irq_mcu_en, afe_dac_con0, irq_cnt);
}


/* hardware implemenataion with audio hardware related function*/
int audio_dsp_hw_open_op(void *user, struct audio_hw_buffer *hw_buf)
{
    struct task_hw_buffer_info *s;
    AudioTask *mtask = (AudioTask *)user;
    int id;


    if (hw_buf == NULL || user == NULL) {
        AUD_LOG_W("%s(), input pointer is NULL\n", __func__);
        return -1;
    }

    if (hw_buf->hw_buffer == false) {
        AUD_LOG_W("%s(), audiohw_buffer is not hwbuffer\n", __func__);
        return -1;
    }

    id = hw_buf->audio_memiftype;

    /* TODO: others parameter check, add to hash table*/

    s = &temp_hw_buffer_info[id];
    s->task_scene_id = mtask->scene;
    memcpy(&s->hw_buf, hw_buf, sizeof(struct audio_hw_buffer));

    /* reset the ring buffer*/
    RingBuf_Map_RingBuf_bridge(&s->hw_buf.aud_buffer.buf_bridge,
                               &s->ring_buf);

    RingBuf_Reset(&s->ring_buf);

    s->id = id;
    s->user = (AudioTask *)user;
    s->state = STATE_IDLE;
    s->prev_idx = s->ring_buf.pBufBase;

#ifdef CFG_AUDIO_SUPPORT
    afe_irq_set_target(s->hw_buf.irq_num, IRQ_TO_HIFI3);
#else
    afe_irq_set_target(s->hw_buf.irq_num, IRQ_TO_SCP);
#endif
    /* move to write*/
    register_dsp_irq(hw_buf->irq_num, (AudioTask *)user);

    AUD_LOG_D("%s(), user(%p), hw_buf %p ,id %d, state %d hw_buf->irq_num[%d]\n",
              __func__, user,
              hw_buf, id, s->state, hw_buf->irq_num);
    return id; /* temp use memtype as unique id */
}


unsigned int audio_dsp_hw_getcur_op(int id)
{
    if (id < 0) {
        return -1;
    }

    struct task_hw_buffer_info *s = &temp_hw_buffer_info[id];

    return afe_memif_current(s->hw_buf.audio_memiftype);
}


int audio_dsp_hw_stop_op(int handle)
{
    if (handle < 0) {
        return -1;
    }

    struct task_hw_buffer_info *s = &temp_hw_buffer_info[handle];

    if (s->state & STATE_RUN) {
        /* stop memif */
        afe_memif_stop(s->hw_buf.audio_memiftype);
        /* stop irq */
        afe_irq_stop(s->hw_buf.irq_num);

        RingBuf_Reset(&s->ring_buf);

        AUD_LOG_D("%s() handle = %d, state(%d), user(%p)\n", __func__, handle, s->state,
                  s->user);
    }
    /* set state to STOP*/
    s->state = STATE_IDLE;

    return 0;
}

int audio_dsp_hw_close_op(int handle)
{
    if (handle < 0) {
        return -1;
    }

    struct task_hw_buffer_info *s = &temp_hw_buffer_info[handle];

    if (s->state & STATE_RUN) {
        audio_dsp_hw_stop_op(handle);
    }

    /* remove from hash table */
    afe_irq_set_target(s->hw_buf.irq_num, IRQ_TO_MCU);

    memset((void *)s, 0, sizeof(struct task_hw_buffer_info));
    s->state = STATE_UNINIT;

    AUD_LOG_D("%s() user(%p), handle(%d), state(%d)\n", __func__, s->user, handle,
              s->state);
    return 0;
}

int audio_dsp_hw_read_op(int handle, char *buf, unsigned int length)
{
    if (handle < 0) {
        return -1;
    }

    struct task_hw_buffer_info *s = &temp_hw_buffer_info[handle];
    struct RingBuf *r_buf = &s->ring_buf;
    struct audio_hw_buffer *hw_buf = &s->hw_buf;
    int ret = 0;
    int datas = 0;
    int bytes = length;
    unsigned int flag = 0;
    unsigned int irq_wait_time =
        get_peroid_mstime(hw_buf->aud_buffer.buffer_attr.rate,
                          hw_buf->aud_buffer.period_size) * 2;
#ifdef DEBUG_VERBOSE
    AUD_LOG_V("%s(), handle(%d), state(%d), length(%d), user(%p) r_buf_count[%d] irq_wait_time[%d] \n",
              __func__, handle, s->state, length, s->user, RingBuf_getDataCount(r_buf), irq_wait_time);
#endif
    if (s->state == STATE_UNINIT) {
        return 0;
    }

    if (s->state & STATE_OVERFLOW) {
        s->state &= ~(STATE_OVERFLOW);
        AUD_LOG_D("%s(), STATE_OVERFLOW, irq_wait_time(%d)\n", __func__, irq_wait_time);
        aud_task_delay(irq_wait_time/2);
        return -1;
    }


    //#define TEST_SGEN_AFE
    //#define TEST_SGEN_DSP_READ
#if defined(TEST_SGEN_AFE)
    /* afe sgen to VUL2 O32/O33*/
    Afe_Set_Reg(AFE_SGEN_CON0, 0x04AE1AE2, 0xffffffff);
    Afe_Set_Reg(AFE_SGEN_CON2, 0x00000032, 0xffffffff); //O32/O33

    AUD_LOG_V("%s()_frog_sgen, AFE_SGEN_CON0(%x), AFE_SGEN_CON2(%x)\n",
              __func__,
              Afe_Get_Reg(AFE_SGEN_CON0),
              Afe_Get_Reg(AFE_SGEN_CON2));
#endif

#if defined(TEST_SGEN_DSP_READ)
    while (ret < length) {
        bytes = length - ret;
        if (bytes >= sizeof(table_sgen_golden_values)) {
            bytes = sizeof(table_sgen_golden_values);
            memcpy(buf + ret, (char *)table_sgen_golden_values, bytes);
        }
        ret += bytes;
    }
    return ret;
#endif

    if (s->state == STATE_IDLE) {
        /* start memif */
        afe_memif_start(hw_buf->audio_memiftype);
        /* start irq */
        if (hw_buf->ignore_irq == false) {
            afe_irq_start(hw_buf->irq_num);
        }
        /* set state to START*/
        s->state = STATE_RUN;
        AUD_LOG_D("%s()_frog_run, state(%d)\n", __func__, s->state);
    }

    while (ret < length) {
        // overflow check
        if (s->state & STATE_OVERFLOW) {
            s->state &= ~(STATE_OVERFLOW);
            AUD_LOG_D("%s(), STATE_OVERFLOW, irq_wait_time(%d)\n", __func__, irq_wait_time);
            aud_task_delay(irq_wait_time/2);
            return -1;
        }

        taskENTER_CRITICAL();
        bytes = length - ret;
        datas = RingBuf_getDataCount(r_buf);

        if (datas < bytes) {
            bytes = datas;
        }

        if (bytes) {
            if (hal_cache_is_cacheable(r_buf->pBufBase)) {
                RingBuf_cache_copyToLinear(buf + ret, r_buf, bytes);
            }
            else {
                RingBuf_copyToLinear(buf + ret, r_buf, bytes);
            }
            ret += bytes;
            taskEXIT_CRITICAL();
        }
        else {
            taskEXIT_CRITICAL();
            flag = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(irq_wait_time));
            if (flag == 0) {
                AUD_LOG_D("%s(), ulTaskNotifyTake handle(%d), state(%d), flag(%d), bytes(%d), datacount(%d)\n",
                          __func__, handle, s->state, flag, bytes, RingBuf_getDataCount(r_buf));
                dump_irq_reg(hw_buf->irq_num);
                dump_mem_reg(hw_buf->memory_type);
            }
        }
    }

    return ret;
}

int audio_dsp_hw_write_op(int handle, char *buf, unsigned int length)
{
    if (handle < 0) {
        AUD_LOG_D("%s() error return -1\n", __func__);
        return -1;
    }

    struct task_hw_buffer_info *s = &temp_hw_buffer_info[handle];
    struct RingBuf *r_buf = &s->ring_buf;
    struct audio_hw_buffer *hw_buf = &s->hw_buf;
    int ret = 0;
    int spaces = 0;
    int bytes = length;
    unsigned int flag = 0;
    unsigned int irq_wait_time =
        get_peroid_mstime(hw_buf->aud_buffer.buffer_attr.rate,
                          hw_buf->aud_buffer.period_size) * 2;
#ifdef DEBUG_VERBOSE
    AUD_LOG_V("+%s(), handle(%d), state(%d), length(%d) freesapce(%d) data(%d) irq_wait_time(%d)\n",
              __func__, handle, s->state, length, RingBuf_getFreeSpace(r_buf),
              RingBuf_getDataCount(r_buf), irq_wait_time);
#endif
    //#define TEST_SGEN_DSP_WRITE
#if defined(TEST_SGEN_DSP_WRITE)
    int i = 0;
    while (i < length) {
        memcpy(buf + i, (char *)table_sgen_golden_values,
               sizeof(table_sgen_golden_values));
        i += sizeof(table_sgen_golden_values);
    }
#endif

    if (s->state == STATE_UNINIT) {
        AUD_LOG_W("%s() s->state == STATE_UNINIT \n", __func__);
        return 0;
    }

    // underflow handle
    if (s->state & STATE_UNDERFLOW) {
        s->state &= ~(STATE_UNDERFLOW);
        AUD_LOG_D("%s(), STATE_UNDERFLOW, irq_wait_time(%d)\n", __func__,
                  irq_wait_time);
        aud_task_delay(irq_wait_time/2);
        return -1;
    }

    while (ret < length || ((length - ret) != 0)) {

        // underflow handle
        if (s->state & STATE_UNDERFLOW) {
            s->state &= ~(STATE_UNDERFLOW);
            AUD_LOG_D("%s(), STATE_UNDERFLOW, irq_wait_time(%d)\n", __func__,
                      irq_wait_time);
            aud_task_delay(irq_wait_time/2);
            return -1;
        }

        taskENTER_CRITICAL();
        bytes = length - ret;
        spaces = RingBuf_getFreeSpace(r_buf);

        if (spaces < bytes) {
            bytes = spaces;
        }

        if (bytes) {
            if (hal_cache_is_cacheable(r_buf->pBufBase)) {
                RingBuf_cache_copyFromLinear(r_buf, buf + ret, bytes);
            }
            else {
                RingBuf_copyFromLinear(r_buf, buf + ret, bytes);
            }
            ret += bytes;
            taskEXIT_CRITICAL();
            /* first write  one time when starting*/
        }
        else {
            taskEXIT_CRITICAL();
            flag = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(irq_wait_time));
            if (flag == 0) {
                AUD_LOG_D("%s(), ulTaskNotifyTake handle(%d), state(%d), flag(%d), spaces(%d), bytes(%d), datacount(%d)\n",
                          __func__, handle, s->state, flag, spaces, bytes, RingBuf_getDataCount(r_buf));
                dump_irq_reg(s->hw_buf.irq_num);
                dump_mem_reg(s->hw_buf.memory_type);
            }
        }

        if ((RingBuf_getDataCount(r_buf) >=
             hw_buf->aud_buffer.start_threshold * getframesize(
                 hw_buf->aud_buffer.buffer_attr.channel,
                 hw_buf->aud_buffer.buffer_attr.format))
            && s->state == STATE_IDLE) {

            /* start memif */
            afe_memif_start(hw_buf->audio_memiftype);
            /* start irq */
            if (hw_buf->ignore_irq == false) {
                afe_irq_start(hw_buf->irq_num);
            }
            /* set state to START*/
            s->state = STATE_RUN;
            AUD_LOG_D("%s()_frog, handle(%d), state(%d), ret(%d), start_threshold(%d), spaces(%d), bytes(%d), datacount(%d)\n",
                      __func__, handle, s->state, ret, hw_buf->aud_buffer.start_threshold, spaces,
                      bytes, RingBuf_getDataCount(r_buf));
        }
    }

    AUD_LOG_V("-%s(), handle(%d), state(%d), length(%d) freesapce(%d) data(%d)\n",
              __func__, handle, s->state, length, RingBuf_getFreeSpace(r_buf),
              RingBuf_getDataCount(r_buf));

    return ret;
}

int audio_dsp_hw_irq_trigger_op(void *user, int handle)
{
    int ret = 0;

    if (handle < 0 || user == NULL) {
        return -1;
    }

    struct task_hw_buffer_info *s = &temp_hw_buffer_info[handle];
    struct RingBuf *r_buf = &s->ring_buf;
    int memif = temp_hw_buffer_info[handle].hw_buf.audio_memiftype;
    unsigned int hwindex;
    AudioTask *task = (AudioTask *)user;

    unsigned int cur_index = afe_memif_current(memif);
    unsigned int databytes = 0;
    int consumebytes = 0;

#ifdef DEBUG_VERBOSE
    AUD_LOG_V("%s() handle = %d, user %p, cur_index = 0x%x is_dl(%d), is_dram(%d)\n",
              __func__,
              handle, user, cur_index , afe_memif_is_dl(memif), s->hw_buf.audio_memiftype);
#endif

    if ( (s->state & STATE_RUN) == 0) {
        AUD_LOG_D("%s() s->state = %d, handle = %d\n",
              __func__, s->state, handle);
        return 0;
    }

    if (afe_memif_is_dl(memif)) {
        s->prev_idx = r_buf->pRead;
        if (s->hw_buf.memory_type) {
            hwindex = ap_to_scp((uint32_t)cur_index);
        }
        else {
            hwindex = SRAM_ADDR_MAPPING(cur_index);
        }

        hwindex = (hwindex & AFE_HW_ALIGNMENT);
        r_buf->pRead = (char *)hwindex;

        if (r_buf->pRead == s->prev_idx) {
            AUD_LOG_V("%s() handle[%d] user[%p] cur_index[0x%x] is_dl[%d] r_buf->pRead[%p] s->prev_idx[%p]\n",
                      __func__, handle, user, cur_index , afe_memif_is_dl(memif), r_buf->pRead,
                      s->prev_idx);
        }

        /* update data count*/
        if (r_buf->pRead > s->prev_idx) {
            consumebytes = r_buf->pRead - s->prev_idx;
        }
        else {
            consumebytes = r_buf->bufLen - (s->prev_idx - r_buf->pRead);
        }

        if (r_buf->datacount - consumebytes < 0) {
            AUD_LOG_W("+%s() underflow consumebytes = %d, r_buf->datacount %d handle(%d)\n",
                      __func__, consumebytes, r_buf->datacount, handle);
            if (r_buf->pWrite >= r_buf->pRead) {
                r_buf->datacount = r_buf->pWrite - r_buf->pRead;
            }
            else {
                r_buf->datacount = r_buf->bufLen - (r_buf->pRead - r_buf->pWrite);
            }
            s->state |= STATE_UNDERFLOW;
            ret = -1;
        }
        else {
            r_buf->datacount -= consumebytes;
        }

    }
    else {
        s->prev_idx = r_buf->pWrite;
        if (s->hw_buf.memory_type) {
            hwindex = ap_to_scp((uint32_t)cur_index);
        }
        else {
            hwindex = SRAM_ADDR_MAPPING(cur_index);
        }

        hwindex = (hwindex & AFE_HW_ALIGNMENT);
        r_buf->pWrite = (char *)hwindex;

        if (r_buf->pWrite == s->prev_idx) {
            AUD_LOG_V("%s() handle[%d] user[%p] cur_index[0x%x] is_dl[%d] r_buf->pWrite[%p s->prev_idx[%p]\n",
                      __func__, handle, user, cur_index , afe_memif_is_dl(memif), r_buf->pWrite,
                      s->prev_idx);
        }

        /* update data count*/
        if (r_buf->pWrite >= s->prev_idx) {
            databytes = r_buf->pWrite - s->prev_idx;
        }
        else {
            databytes = r_buf->bufLen - (s->prev_idx - r_buf->pWrite);
        }

        if (r_buf->datacount + databytes >= r_buf->bufLen) {
            AUD_LOG_W("%s() overflow databytes = %d r_buf->datacount %d handle(%d)\n",
                      __func__, databytes, r_buf->datacount, handle);
            if (r_buf->pWrite >= r_buf->pRead) {
                r_buf->datacount = r_buf->pWrite - r_buf->pRead;
            }
            else {
                r_buf->datacount = r_buf->bufLen - (r_buf->pRead - r_buf->pWrite);
            }
            s->state |= STATE_OVERFLOW;
            ret = -1;
        }
        else {
            r_buf->datacount += databytes;
        }
    }
#ifdef DEBUG_VERBOSE
    AUD_LOG_V("-%s() handle = %d, user %p, cur_index = 0x%x is_dl(%d), audio_memiftype(%d) datacount[%d] databytes[%d]\n",
              __func__,
              handle, user, cur_index,
              afe_memif_is_dl(memif),
              s->hw_buf.audio_memiftype,
              RingBuf_getDataCount(r_buf),
              databytes);
#endif

    /* wake up task which block in write */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(task->freertos_task, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    return ret;
}

int audio_dsp_hw_getlen_op(int id)
{
    if (id < 0) {
        return -1;
    }

    struct task_hw_buffer_info *s = &temp_hw_buffer_info[id];

    return s->ring_buf.bufLen;
}

int audio_dsp_hw_getavail_op(int handle)
{
    if (handle < 0) {
        return -1;
    }

    struct task_hw_buffer_info *s = &temp_hw_buffer_info[handle];

    return s->ring_buf.datacount;
}

int audio_dsp_hw_status_op(void *user, struct audio_hw_buffer *hw_buf)
{
    int id = hw_buf->audio_memiftype;
    struct task_hw_buffer_info *s;

    if (id < 0) {
        return -1;
    }

    s = &temp_hw_buffer_info[id];
    return s->state;
}

int audio_dsp_hw_trigger_op(int handle, int cmd)
{
    if (handle < 0) {
        return -1;
    }

    struct task_hw_buffer_info *s = &temp_hw_buffer_info[handle];

    if (cmd == TRIGGER_START) {
        /* start memif */
        afe_memif_start(s->hw_buf.audio_memiftype);
        /* start irq */
        if (s->hw_buf.ignore_irq == false) {
            afe_irq_start(s->hw_buf.irq_num);
        }
        AUD_LOG_D("%s() state(%d) handle[%d]\n", __func__, s->state, handle);
    }
    else if (cmd == TRIGGER_STOP) {
        /* stop memif */
        afe_memif_stop(s->hw_buf.audio_memiftype);
        /* stop irq */
        if (s->hw_buf.ignore_irq == false) {
            afe_irq_stop(s->hw_buf.irq_num);
        }
        AUD_LOG_D("%s() state(%d) handle[%d]\n", __func__, s->state, handle);
    }
    else {
        AUD_LOG_W("%s() warn state(%d) handle[%d]\n", __func__, s->state, handle);
    }
    return 0;
}

int audio_dsp_hw_update_ptr_op(int handle)
{
    if (handle < 0) {
        return -1;
    }

    struct task_hw_buffer_info *s = &temp_hw_buffer_info[handle];
    struct RingBuf *r_buf = &s->ring_buf;
    int memif = temp_hw_buffer_info[handle].hw_buf.audio_memiftype;
    unsigned int hwindex;

    unsigned int cur_index = afe_memif_current(memif);
    unsigned int databytes = 0;
    int consumebytes = 0;

    if ((s->state & STATE_RUN) < 0) {
        return 0;
    }

    taskENTER_CRITICAL();
    if (afe_memif_is_dl(memif)) {
        s->prev_idx = r_buf->pRead;
        if (s->hw_buf.memory_type) {
            hwindex = ap_to_scp((uint32_t)cur_index);
        }
        else {
            hwindex = SRAM_ADDR_MAPPING(cur_index);
        }

        hwindex = (hwindex & AFE_HW_ALIGNMENT);
        r_buf->pRead = (char *)hwindex;

        if (r_buf->pRead == s->prev_idx) {
            AUD_LOG_V("%s() handle[%d] user[%p] cur_index[0x%x] is_dl[%d] r_buf->pRead[%p] s->prev_idx[%p]\n",
                      __func__, handle, user, cur_index , afe_memif_is_dl(memif), r_buf->pRead,
                      s->prev_idx);
        }

        /* update data count*/
        if (r_buf->pRead > s->prev_idx) {
            consumebytes = r_buf->pRead - s->prev_idx;
        }
        else {
            consumebytes = r_buf->bufLen - (s->prev_idx - r_buf->pRead);
        }

        if (r_buf->datacount - consumebytes < 0) {
            AUD_LOG_W("+%s() underflow consumebytes = %d, r_buf->datacount %d handle(%d)\n",
                      __func__, consumebytes, r_buf->datacount, handle);
            if (r_buf->pWrite >= r_buf->pRead) {
                r_buf->datacount = r_buf->pWrite - r_buf->pRead;
            }
            else {
                r_buf->datacount = r_buf->bufLen - (r_buf->pRead - r_buf->pWrite);
            }
        }
        else {
            r_buf->datacount -= consumebytes;
        }

    }
    else {
        s->prev_idx = r_buf->pWrite;
        if (s->hw_buf.memory_type) {
            hwindex = ap_to_scp((uint32_t)cur_index);
        }
        else {
            hwindex = SRAM_ADDR_MAPPING(cur_index);
        }

        hwindex = (hwindex & AFE_HW_ALIGNMENT);
        r_buf->pWrite = (char *)hwindex;

        if (r_buf->pWrite == s->prev_idx) {
            AUD_LOG_D("%s() handle[%d] cur_index[0x%x] is_dl[%d] r_buf->pWrite[%p] s->prev_idx[%p]\n",
                      __func__, handle, cur_index , afe_memif_is_dl(memif), r_buf->pWrite,
                      s->prev_idx);
        }

        /* update data count*/
        if (r_buf->pWrite >= s->prev_idx) {
            databytes = r_buf->pWrite - s->prev_idx;
        }
        else {
            databytes = r_buf->bufLen - (s->prev_idx - r_buf->pWrite);
        }

        if (r_buf->datacount + databytes >= r_buf->bufLen) {
            AUD_LOG_W("%s() overflow databytes = %d r_buf->datacount %d r_buf->pWrite[%p] s->prev_idx[%p] cur_index[0x%x] hwindex[0x%x]\n",
                      __func__, databytes, r_buf->datacount, r_buf->pWrite, s->prev_idx, cur_index, hwindex);
            if (r_buf->pWrite >= r_buf->pRead) {
                r_buf->datacount = r_buf->pWrite - r_buf->pRead;
            }
            else {
                r_buf->datacount = r_buf->bufLen - (r_buf->pRead - r_buf->pWrite);
            }
        }
        else {
            r_buf->datacount += databytes;
        }
    }
    taskEXIT_CRITICAL();

    return 0;
}


static struct audio_dsp_hw_hal_ops s_audio_dsp_hw_hal_ops = {
    .audio_dsp_hw_open_op = audio_dsp_hw_open_op,
    .audio_dsp_hw_read_op = audio_dsp_hw_read_op,
    .audio_dsp_hw_close_op = audio_dsp_hw_close_op,
    .audio_dsp_hw_write_op = audio_dsp_hw_write_op,
    .audio_dsp_hw_stop_op = audio_dsp_hw_stop_op,
    .audio_dsp_hw_getcur_op = audio_dsp_hw_getcur_op,
    .audio_dsp_hw_getlen_op = audio_dsp_hw_getlen_op,
    .audio_dsp_hw_getavail_op = audio_dsp_hw_getavail_op,
    .audio_dsp_hw_status_op = audio_dsp_hw_status_op,
    .audio_dsp_hw_trigger_op = audio_dsp_hw_trigger_op,
    .audio_dsp_hw_update_ptr_op = audio_dsp_hw_update_ptr_op,
};

static struct audio_dsp_hw_ops s_audio_dsp_hw_ops = {
    .audio_dsp_hw_irq_op = audio_dsp_hw_irq_trigger_op,
};


static int set_dsp_audio_hw_hal_ops(void)
{
    return set_audio_dsp_hw_hal_ops(&s_audio_dsp_hw_hal_ops);
}

static int set_dsp_audio_hw_ops(void)
{
    return set_audio_dsp_hw_ops(&s_audio_dsp_hw_ops);
}

int init_dsp_audio_ops(void)
{
    int ret = 0;
    ret = set_dsp_audio_hw_hal_ops();
    if (ret < 0) {
        AUD_LOG_E("%s set_dsp_audio_hw_hal_ops err\n", __func__);
        return ret;
    }

    ret = set_dsp_audio_hw_ops();
    if (ret < 0) {
        AUD_LOG_E("%s set_dsp_audio_hw_ops err\n", __func__);
        return ret;
    }

    return 0;
}


