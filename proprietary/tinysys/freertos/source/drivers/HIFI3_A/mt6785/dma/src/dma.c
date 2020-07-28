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

#include "main.h"
/*   Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <driver_api.h>
#include <dma.h>
#include <string.h>
#include <interrupt.h>
#include <platform_mtk.h>
#ifdef CFG_XGPT_SUPPORT
#include <mt_gpt.h>
#endif
#ifdef CFG_MPU_DEBUG_SUPPORT
#include "mpu_mtk.h"
#endif
#if DMA_KEEP_AWAKE
#include <wakelock.h>
#endif


static int semaphore_count = 0;
/*
 * global variables
 */
struct dma_id_ctrl_t dma_id_ctrl[NUMBER_DMA_ID];
struct dma_channel_t dma_channel[NR_GDMA_CHANNEL];
#if DMA_QUEUE_SIZE > 0
struct dma_list_ctrl_t dma_list_ctrl[DMA_QUEUE_SIZE];
#endif
#if DMA_KEEP_AWAKE
wakelock_t scp_dma_wakelock;
#endif

#if DMA_QUEUE_SIZE > 0
/*dma list current list item*/
uint32_t scp_dma_list_current;
/*dma list*/
static List_t xScpDmaList;
#endif
typedef struct DMAlist {

} DMAlist_t;


/*
 * get_emi_semaphore: must get emi resource before access emi
 */
/* Althoug it will access infra bus here.
 * There is no need to enable infra clock here,
 * the infra clock already enable in prevous function call.
 * */
void get_emi_semaphore()
{

/*
 * there is no need to use emi semaphore
 * after mt6799, reserved for debugging
 */
#ifdef CFG_MPU_DEBUG_SUPPORT
    disable_dram_protector();
#endif
    semaphore_count ++;
}

/*
 * release_emi_semaphore: must release emi resource after emi access
 */
/* Althoug it will access infra bus here.
 * There is no need to enable infra clock here,
 * the infra clock already enable in prevous function call.
 * */
void release_emi_semaphore()
{

/*
 * there is no need to use emi semaphore
 * after mt6799, reserved Debugging
 */
    semaphore_count --;
#ifdef CFG_MPU_DEBUG_SUPPORT
    enable_dram_protector();
#endif

}

/*
 * mt_req_gdma: request a general DMA.
 * @chan: specify a channel or not
 * Return channel number for success; return negative error code for failure.
 */
int32_t mt_req_gdma(DMA_CHAN chan)
{
    int32_t i;
    //dbgmsg("GDMA_%d - %s\n", "mt_req_gdma", chan);

    if (chan >= (NR_GDMA_CHANNEL)) {
        return DMA_RESULT_INVALID_CH;
    }

    if (!is_in_isr())
        taskENTER_CRITICAL();

    if (dma_channel[chan].in_use) {
        PRINTF_E("[DMA]ch%d in use\n", chan);
        if (!is_in_isr())
            taskEXIT_CRITICAL();
        return -DMA_RESULT_RUNNING;
    } else {
        i = chan;
        dma_channel[chan].in_use = 1;
    }

    if (!is_in_isr())
        taskEXIT_CRITICAL();

    mt_reset_gdma_conf(i);
    return i;
}

/*
 * mt_check_dma_channel: check DMA channel whether in use or not
 * @chan: specify a channel or not
 * Return 0 channel no use
 * Return negative value identify error code
 */
int32_t mt_check_dma_channel(DMA_CHAN chan)
{
    int32_t i;
    //dbgmsg("GDMA_%d - %s\n", "mt_check_dma_channel", chan);

    if (chan >= (NR_GDMA_CHANNEL)) {
        return DMA_RESULT_INVALID_CH;
    }

    if (!is_in_isr())
        taskENTER_CRITICAL();

    if (dma_channel[chan].in_use) {
        PRINTF_E("[DMA]ch %d in use\n", chan);
        if (!is_in_isr())
            taskEXIT_CRITICAL();
        return -DMA_RESULT_RUNNING;
    } else {
        i = chan;
    }

    if (!is_in_isr())
        taskEXIT_CRITICAL();

    return i;
}

/*
 * mt_start_gdma: start the DMA stransfer for the specified GDMA channel
 * @channel: GDMA channel to start
 * Return 0 for success; return negative errot code for failure.
 */
int32_t mt_start_gdma(int32_t channel)
{
    uint32_t enable_channel;

    enable_channel = DMA_BASE_CH(channel);

    if ((channel < GDMA_START) || (channel >= (GDMA_START + NR_GDMA_CHANNEL))) {
        return DMA_RESULT_INVALID_CH;
    }

    //dbgmsg("[DMA]ch%d-start\n", channel);
    drv_write_reg32(DMA_ACKINT(enable_channel), DMA_ACK_BIT);
    drv_write_reg32(DMA_START(enable_channel), DMA_START_BIT);

    return 0;
}

/*
 * mt_polling_gdma: wait the DMA to finish for the specified GDMA channel
 * @channel: GDMA channel to polling
 * @timeout: polling timeout in ms
 * Return 0 for success; return negative errot code for failure.
 */
int32_t mt_polling_gdma(int32_t channel, uint32_t timeout)
{
    if (channel < GDMA_START) {
        return DMA_RESULT_INVALID_CH;
    }

    if (channel >= (GDMA_START + NR_GDMA_CHANNEL)) {
        return DMA_RESULT_INVALID_CH;
    }

    if (dma_channel[channel].in_use == 0) {
        return DMA_RESULT_CH_FREE;
    }

    //dbgmsg("[DMA]ch:%d-%s\n", channel, "mt_polling_gdma");

    //timeout = 0x10000;//jiffies + ((HZ * timeout) / 1000);

    /*dbgmsg("[DMA]%ld: GBLSTA:0x%x\n", drv_reg32(DMA_GLBSTA));*/
    while ((drv_reg32(DMA_GLBSTA) & DMA_GLBSTA_RUN(channel))) {
        timeout--;
        if (!timeout) {
            PRINTF_E("DMA%d polling timeout!!!!\n", channel + 1);
            return 1;
        }
    }

    return 0;
}

/*
 * mt_stop_gdma: stop the DMA stransfer for the specified GDMA channel
 * @channel: GDMA channel to stop
 * Return 0 for success; return negative errot code for failure.
 */
int32_t mt_stop_gdma(int32_t channel)
{
    uint32_t stop_channel;

    stop_channel = DMA_BASE_CH(channel);
    if (channel < GDMA_START) {
        return DMA_RESULT_INVALID_CH;
    }

    if (channel >= (GDMA_START + NR_GDMA_CHANNEL)) {
        return DMA_RESULT_INVALID_CH;
    }

    if (dma_channel[channel].in_use == 0) {
        PRINTF_E("%s(), DMA%d ch is not in use\n", __func__, (int)channel);
        //return DMA_RESULT_CH_FREE;
    }

    /*dbgmsg("[DMA]ch%d-stop\n", channel);*/

    drv_write_reg32(DMA_ACKINT(stop_channel), DMA_ACK_BIT);
    drv_write_reg32(DMA_START(stop_channel), DMA_START_CLR_BIT);

    return 0;
}

/*
 * mt_config_gdma: configure the given GDMA channel.
 * @channel: GDMA channel to configure
 * @config: pointer to the mt_gdma_conf structure in which the GDMA configurations store
 * @flag: ALL, SRC, DST, or SRC_AND_DST.
 * Return 0 for success; return negative errot code for failure.
 */
int32_t mt_config_gdma(int32_t channel, struct mt_gdma_conf *config, DMA_CONF_FLAG flag)
{
    uint32_t dma_con = 0x0;
    uint32_t config_channel;

    config_channel = DMA_BASE_CH(channel);

    if ((channel < GDMA_START) || (channel >= (GDMA_START + NR_GDMA_CHANNEL))) {
        PRINTF_E("DMA%d ch err\n", (int)channel);
        return DMA_RESULT_INVALID_CH;
    }

    if (dma_channel[channel].in_use == 0) {
        PRINTF_E("DMA%d in_use=0\n", (int)channel);
        return DMA_RESULT_CH_FREE;
    }

    if (!config) {
        PRINTF_E("DMA%d cfg null\n", (int)channel);
        return DMA_RESULT_ERROR;
    }

    if (config->count > MAX_TRANSFER_LEN) {
        PRINTF_E("DMA%d count:0x%x over 0x%x\n", (int)channel, config->count, MAX_TRANSFER_LEN);
        return DMA_RESULT_ERROR;
    }

    if (config->count < 0) {
        PRINTF_E("DMA%d count:0x%x err\n", (int)channel, config->count);
        return DMA_RESULT_ERROR;
    }

    if (config->limiter) {
        PRINTF_E("DMA%d counter over 0x%x\n", (int)channel, MAX_SLOW_DOWN_CNTER);
        return DMA_RESULT_ERROR;
    }

    switch (flag) {
        case ALL:
            /* Control Register */
            drv_write_reg32(DMA_SRC(config_channel), config->src);
            drv_write_reg32(DMA_DST(config_channel), config->dst);
            drv_write_reg32(DMA_COUNT(config_channel), (config->count & MAX_TRANSFER_LEN));

            if (config->wpen) {
                //dbgmsg("wpen\n");
                dma_con |= DMA_CON_WPEN;
            }
            if (config->wpsd) {
                //dbgmsg("wpsd\n");
                dma_con |= DMA_CON_WPSD;
            }
            if (config->wplen) {
                //dbgmsg("wplen\n");
                drv_write_reg32(DMA_WPPT(config_channel), config->wplen);
            }
            if (config->wpto) {
                //dbgmsg("wpto\n");
                drv_write_reg32(DMA_WPTO(config_channel), config->wpto);
            }

            dma_con |= (config->sinc << DMA_CON_SINC);
            dma_con |= (config->dinc << DMA_CON_DINC);
            dma_con |= (config->size_per_count << DMA_CON_SIZE);
            dma_con |= (config->burst << DMA_CON_BURST);
            dma_con |= (config->iten << DMA_CON_ITEN);

            drv_write_reg32(DMA_CON(config_channel), dma_con);
            //dbgmsg("[DMA]ch:%d src:0x%x, dst:0x%x, count:0x%x, con:0x%x\n\r", channel, config->src, config->dst, config->count, dma_con);
            break;

        case SRC:
            drv_write_reg32(DMA_SRC(config_channel), config->src);

            break;

        case DST:
            drv_write_reg32(DMA_DST(config_channel), config->dst);
            break;

        case SRC_AND_DST:
            drv_write_reg32(DMA_SRC(config_channel), config->src);
            drv_write_reg32(DMA_DST(config_channel), config->dst);
            break;

        default:
            break;
    }

    /* use the data synchronization barrier to ensure that all writes are completed */
    //dsb();

    return 0;
}


/*
 * mt_free_gdma: free a general DMA.
 * @channel: channel to free
 * Return 0 for success; return negative errot code for failure.
 */
int32_t mt_free_gdma(DMA_CHAN chan)
{

    if (chan < GDMA_START) {
        return DMA_RESULT_INVALID_CH;
    }

    if (chan >= (GDMA_START + NR_GDMA_CHANNEL)) {
        return DMA_RESULT_INVALID_CH;
    }

    if (dma_channel[chan].in_use == 0) {
        return DMA_RESULT_CH_FREE;
    }

    /*dbgmsg("[DMA]ch%d-free\n", chan);*/

    mt_stop_gdma(chan);

    if (!is_in_isr())
        taskENTER_CRITICAL();

    dma_channel[chan].in_use = 0;

#if DMA_KEEP_AWAKE
    uint32_t scp_sleep_flag;
    int32_t channel;
    /*scp_sleep_flag :0 release lock, 1:can not release lock*/
    scp_sleep_flag = 0;
    /*scan dma channel scp sleep or not*/
    for (channel = NR_GDMA_CHANNEL-1; channel >= 0; channel--) {
        if(dma_channel[channel].in_use == 1)
            scp_sleep_flag = 1;
    }
    if(scp_sleep_flag == 0){
        /*scp release wakelock*/
        if (!is_in_isr())
            wake_unlock(&scp_dma_wakelock);
        else
            wake_unlock_FromISR(&scp_dma_wakelock);
    }
#endif

    if (!is_in_isr())
        taskEXIT_CRITICAL();

    return 0;
}

/*
 * mt_dump_gdma: dump registers for the specified GDMA channel
 * @channel: GDMA channel to dump registers
 * Return 0 for success; return negative errot code for failure.
 */
int32_t mt_dump_gdma(int32_t channel)
{
    uint32_t i, reg;
    uint32_t j = 0;

    //dbgmsg("GDMA> reg_dump 0x%X=\n\t", DMA_BASE_CH(channel));
    for (i = 0; i <= GDMA_REG_BANK_SIZE; i += 4) {
        reg = drv_reg32(DMA_BASE_CH(channel) + i);
        PRINTF_D("0x%x ", reg);

        if (j++ >= 3) {
            //dbgmsg("\n");
            //dbgmsg("GDMA> reg_dump 0x%X=\n\t", DMA_BASE_CH(channel) + i + 4);
            j = 0;
        }
    }
    return 0;
}



/*
 * mt_request_dma_channel: request free DMA channel,
 * Return 0~X for get DMA free channel;
 * Return DMA_RESULT_NO_FREE_CH info. no free channel;
 */
int32_t mt_request_dma_channel(DMA_ID scp_dma_id)
{
    int32_t i;
    int32_t free_channel;

    free_channel = DMA_RESULT_NO_FREE_CH;
    for (i = NR_GDMA_CHANNEL-1 ; i >= 0; i--) {
        /*skip reserved channel*/
        if(RESERVED_DMA_CHANNEL & 0x1 << i)
           continue;

        if(!is_in_isr())
            taskENTER_CRITICAL();

        /*get free channel*/
        if(dma_channel[i].in_use == 0){
            free_channel = i;
            dma_id_ctrl[scp_dma_id].in_use = DMA_ID_RUNNING;
            dma_id_ctrl[scp_dma_id].dma_channel_num = free_channel;
            dma_id_ctrl[scp_dma_id].dma_usage_count++;
            dma_channel[i].in_use = 1;
            dma_channel[i].ch_usage_count++;
        }

        if(!is_in_isr())
            taskEXIT_CRITICAL();

        if(free_channel >= 0) {
            /*some dma id keep channel, keep scp awake*/
#if DMA_KEEP_AWAKE
            if(!is_in_isr())
                wake_lock(&scp_dma_wakelock);
            else
                wake_lock_FromISR(&scp_dma_wakelock);
#endif
            break;
        }

    }

    return free_channel;
}

/*
 * mt_reset_gdma_conf: reset the config of the specified DMA channel
 * @iChannel: channel number of the DMA channel to reset
 */
void mt_reset_gdma_conf(const uint32_t iChannel)
{
    struct mt_gdma_conf conf;

    //dbgmsg("[DMA]ch:%d-%s\n", iChannel, "mt_reset_gdma_conf");

    memset(&conf, 0, sizeof(struct mt_gdma_conf));

    if (mt_config_gdma(iChannel, &conf, ALL) != 0) {
        return;
    }

    return;
}

static void mt_dma_irqhandler(void)
{
    volatile unsigned glbsta = drv_reg32(DMA_GLBSTA);
    BaseType_t xHigherPriorityTaskWoken;
    //PRINTF_E("[DMA isr]GLBSTA:0x%x\n\r", glbsta);
    int32_t channel;
    int32_t dma_id;
    uint64_t duration;

    xHigherPriorityTaskWoken = pdFALSE;

    /*scan all channel status*/
    for (channel = NR_GDMA_CHANNEL-1; channel >= 0; channel--) {
        if (glbsta & (0x2 << (channel * 2))) {
            //dbgmsg("[DMA isr]in isr ch:%d\n\r", channel);
            /*scan all dma id*/
            for (dma_id = 0; dma_id < NUMS_DMA_ID; dma_id++) {
                if ((dma_id_ctrl[dma_id].dma_channel_num == channel) && (dma_id_ctrl[dma_id].in_use == DMA_ID_RUNNING)){
                    /*execute dma isr_cb*/
                    if (dma_id_ctrl[dma_id].isr_cb){
#ifdef CFG_XGPT_SUPPORT
                        dma_id_ctrl[dma_id].last_enter = read_xgpt_stamp_ns();
#endif
                        /*execute dma isr*/
                        dma_id_ctrl[dma_id].isr_cb(dma_id_ctrl[dma_id].data);
#ifdef CFG_XGPT_SUPPORT
                        dma_id_ctrl[dma_id].last_exit = read_xgpt_stamp_ns();
#endif
                        duration = dma_id_ctrl[dma_id].last_exit - dma_id_ctrl[dma_id].last_enter;
                        /* handle the xgpt overflow case
                        * discard the duration time when exit time < enter time
                        * */
                        if (dma_id_ctrl[dma_id].last_exit > dma_id_ctrl[dma_id].last_enter){
                            if (duration > dma_id_ctrl[dma_id].max_duration)
                                dma_id_ctrl[dma_id].max_duration = duration;
                        }
                    }/*end cb execute*/

                    /*clear dma in use*/
                    mt_free_gdma(channel);
                    //mt_free_dma_ctrl(dma_id);

                    /* resume task here to make sure response*/
                    xSemaphoreGiveFromISR( dma_id_ctrl[dma_id].xDMASemaphore, &xHigherPriorityTaskWoken );

                }/*end id == channel check*/
            }/*end each dma id check*/

        } else {
            //PRINTF_E("[DMA] discard interrupt\n");
        }
    }/*end scan all dma channel status*/

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    //PRINTF_E("[DMA isr]exit\n\r");
}

/*
 * mt_set_dma_ctrl: mt_set_dma_ctrl
 * return 0  :set success
 * return DMA_RESULT_RUNNING :set ctrl fail
 */
int32_t mt_set_dma_ctrl(DMA_ID scp_dma_id, DMA_ID_STATUS scp_dma_id_status)
{
    int32_t ret;
    ret = 0;
    if (scp_dma_id < 0 || scp_dma_id >= NUMS_DMA_ID) {
        PRINTF_E("DMA ID err\n");
        return DMA_RESULT_ERROR;
    }
    if(!is_in_isr())
        taskENTER_CRITICAL();

    if(dma_id_ctrl[scp_dma_id].in_use == DMA_ID_RUNNING) {
        PRINTF_E("[DMA]id:%d already using\n", scp_dma_id);
        /*dump dma info here*/
        //mt_dump_dma_struct();
        //configASSERT(0);
        ret = DMA_RESULT_RUNNING;
    }
    dma_id_ctrl[scp_dma_id].in_use = scp_dma_id_status;

    if(!is_in_isr())
        taskEXIT_CRITICAL();

    return ret;
}

/*
 * mt_free_dma_ctrl: mt_free_dma_ctrl
 * return 0  :free success
 * return -1 :free ctrl fail
 */
int32_t mt_free_dma_ctrl(DMA_ID scp_dma_id)
{
    int32_t ret;
    ret = 0;
    if (scp_dma_id < 0 || scp_dma_id >= NUMS_DMA_ID) {
        PRINTF_E("DMA ID err\n");
        return DMA_RESULT_ERROR;
    }
    if(!is_in_isr())
        taskENTER_CRITICAL();

    if(dma_id_ctrl[scp_dma_id].in_use == DMA_ID_RUNNING){
            dma_id_ctrl[scp_dma_id].in_use = DMA_ID_NO_USE;
            dma_id_ctrl[scp_dma_id].dma_channel_num = -1;
    } else {
        /*free unused dma ctrl*/
        PRINTF_E("[DMA]id:%d free dma err\n", scp_dma_id);
        /*dump dma info here*/
        configASSERT(0);
        ret = DMA_RESULT_ERROR;
    }

    if(!is_in_isr())
        taskEXIT_CRITICAL();

    return ret;
}

/*
 * mt_request_dma_cb: mt_request_dma_cb
 * return 0  :regi success
 * return -1 :regi fail
 */
int32_t scp_dma_registration(DMA_ID scp_dma_id, void (*isr_cb)(void *))
{
    if (scp_dma_id < 0 || scp_dma_id >= NUMS_DMA_ID) {
        PRINTF_E("DMA ID err\n");
        return DMA_RESULT_ERROR;
    }
    if(!is_in_isr())
        taskENTER_CRITICAL();

    dma_id_ctrl[scp_dma_id].isr_cb = isr_cb;

    if(!is_in_isr())
        taskEXIT_CRITICAL();

    return 0;
}

/*
 * mt_dump_dma_struct: dump DMA struct info
 */
void mt_dump_dma_struct()
{
    uint32_t i;

    for (i = 0; i < NUMS_DMA_ID; i ++) {
        if (dma_id_ctrl[i].in_use != DMA_ID_NO_USE|| dma_id_ctrl[i].dma_usage_count > 0) {
            PRINTF_E("dma_ctrl id:%d,use:%d,ch:%d,usage:%u\n", i, dma_id_ctrl[i].in_use
            , dma_id_ctrl[i].dma_channel_num, dma_id_ctrl[i].dma_usage_count);
        }
    }
    for (i = 0; i < NR_GDMA_CHANNEL; i ++) {
        if (dma_channel[i].in_use != 0 || dma_channel[i].ch_usage_count > 0) {
            PRINTF_E("dma ch:%d,use:%d,count:%u\n", i,dma_channel[i].in_use,dma_channel[i].ch_usage_count);
        }
    }
    return;
}

/*
 * mt_init_dma: initialize DMA.
 * Always return 0.
 */
int32_t mt_init_dma(void)
{
    int32_t i;
    for (i = 0; i < NR_GDMA_CHANNEL; i++) {
        dma_channel[i].in_use = 0;
        mt_reset_gdma_conf(i);
        mt_stop_gdma(i);
    }
    for (i = 0; i < NUMS_DMA_ID; i++) {
        dma_id_ctrl[i].in_use = DMA_ID_NO_USE;
        dma_id_ctrl[i].dma_channel_num = -1;
        dma_id_ctrl[i].isr_cb = NULL;
        dma_id_ctrl[i].xDMASemaphore = xSemaphoreCreateBinary();
    }
#if DMA_QUEUE_SIZE > 0
    for (i = 0; i < DMA_QUEUE_SIZE; i++) {
        /* initialize lock structure: make sure the item is not on a list.*/
        vListInitialiseItem(&(dma_list_ctrl[i].xDMAListItem));
        dma_list_ctrl[i].dma_id_list = -1;
        dma_list_ctrl[i].dst_addr_list = 0;
        dma_list_ctrl[i].src_addr_list = 0;
        dma_list_ctrl[i].len_list = 0;
    }
#endif

#if DMA_QUEUE_SIZE > 0
    scp_dma_list_current = 0;
    vListInitialise(&xScpDmaList);
#endif
    request_irq(DMA_IRQn, mt_dma_irqhandler, "DMA");
#if DMA_KEEP_AWAKE
    wake_lock_init(&scp_dma_wakelock,"dmawk");
#endif

    return 0;
}
#if DMA_QUEUE_SIZE > 0
/*
 * mt_dma_list_insert: insert DMA req to lsit
 * return 0 insert list success
 * return -1 insert list fail
 */
int32_t mt_dma_list_insert(uint32_t dst_addr, uint32_t src_addr, uint32_t len, DMA_ID scp_dma_id)
{
    if(!is_in_isr())
        taskENTER_CRITICAL();

    if((listCURRENT_LIST_LENGTH(&xScpDmaList) >= DMA_QUEUE_SIZE)){
        //PRINTF_E("[DMA]QUEUE overflow\n\r");
        if(!is_in_isr())
            taskEXIT_CRITICAL();
        return -1;
    }

    /*ring buffer*/
    if (scp_dma_list_current >= DMA_QUEUE_SIZE) {
        scp_dma_list_current = 0;
    }

    vListInsertEnd(&xScpDmaList, &dma_list_ctrl[scp_dma_list_current].xDMAListItem);
    dma_list_ctrl[scp_dma_list_current].dma_id_list = scp_dma_id;
    dma_list_ctrl[scp_dma_list_current].dst_addr_list = dst_addr;
    dma_list_ctrl[scp_dma_list_current].src_addr_list = src_addr;
    dma_list_ctrl[scp_dma_list_current].len_list = len;
    dma_list_ctrl[scp_dma_list_current].in_list = 1;
    listSET_LIST_ITEM_VALUE(&(dma_list_ctrl[scp_dma_list_current].xDMAListItem),scp_dma_list_current);
    scp_dma_list_current++;


    if(!is_in_isr())
        taskEXIT_CRITICAL();

    return 0;
}


/*
 * mt_dma_list_remove: remove and return dma list
 * return list iten value.
 * return DMA_QUEUE_SIZE means list empty.
 */
uint32_t mt_dma_list_remove(void)
{
    ListItem_t *pxDMAListItem;
    uint32_t DmaListItem_value;
    DmaListItem_value = DMA_QUEUE_SIZE;
    if (listLIST_IS_EMPTY(&xScpDmaList) != pdFALSE) {
        //PRINTF_E("[DMA]list is empty\n\r");
    } else {

        if(!is_in_isr())
            taskENTER_CRITICAL();

        /* list not empty*/
        pxDMAListItem = listGET_HEAD_ENTRY(&xScpDmaList);
        DmaListItem_value = (uint32_t)listGET_LIST_ITEM_VALUE(pxDMAListItem);
        dma_list_ctrl[DmaListItem_value].in_list = 0;
        uxListRemove(pxDMAListItem);

        if(!is_in_isr())
            taskEXIT_CRITICAL();
    }

    //PRINTF_E("[DMA]list value =%u, scp_dma_list_current=%u\n\r", DmaListItem_value ,scp_dma_list_current);

    return DmaListItem_value;
}

/*
 * mt_dump_dma_list: dump dma list info
 * Always return 0.
 */
void mt_dump_dma_list(void)
{
    int32_t i;
    for (i = 0; i < DMA_QUEUE_SIZE; i++) {
        //PRINTF_E("[SCP]DMAlistctrl[%d]id=%u,dst=%lu,src=%lu,len=%lu,in_list=%lu\n\r", i,dma_list_ctrl[i].dma_id_list,dma_list_ctrl[i].dst_addr_list
        ,dma_list_ctrl[i].src_addr_list,dma_list_ctrl[i].len_list,dma_list_ctrl[i].in_list);
    }
    //PRINTF_E("[SCP]DMA list length=%u\n\r",listCURRENT_LIST_LENGTH(&xScpDmaList));
}
#endif // #if DMA_QUEUE_SIZE > 0
