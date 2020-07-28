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

#ifndef __CCCI_H__
#define __CCCI_H__

#if defined(CFG_FLP_SUPPORT) && !defined(CFG_GEOFENCE_SUPPORT)
#include <sensor_manager.h>
#else
#include <stdint.h>
#endif
#include <ccci_platform.h>

#define __PACKED __attribute__((packed))

#if defined(CFG_FLP_SUPPORT) && !defined(CFG_GEOFENCE_SUPPORT)
#define CCCI_SENSOR_SUPPORT
#endif
//#define CCCI_PROFILING
#define CCCI_MTU 400

enum {
    MD_SYS1 = 0,
    MD_SYS3 = 1, // FIXME, convert on SCP side !!!
    MD_NUM,
};

enum ccci_ipi_op_id {
    CCCI_OP_SCP_STATE,
    CCCI_OP_MD_STATE,
    CCCI_OP_SHM_INIT,
    CCCI_OP_SHM_RESET,

    CCCI_OP_LOG_LEVEL,
    CCCI_OP_GPIO_TEST,
    CCCI_OP_EINT_TEST,
    CCCI_OP_MSGSND_TEST,
    CCCI_OP_ASSERT_TEST,
};

enum {
    MD_BOOT_STAGE_0 = 0,
    MD_BOOT_STAGE_1 = 1,
    MD_BOOT_STAGE_2 = 2,
    MD_BOOT_STAGE_EXCEPTION = 3,
};

enum {
    SCP_CCCI_STATE_INVALID = 0,
    SCP_CCCI_STATE_BOOTING = 1, // common, always send from MD1
    SCP_CCCI_STATE_RBREADY = 2, // per modem
};

/*
 * unlike AP CCCI, here we don't have any channel<->queue mapping, all hard code!
 * C2K's channel number is different from MD1's.
 * tx and rx must share the same queue.
 */

enum {
    CCCI_CELLINFO_CHANNEL_RX = 0x0,
    CCCI_CELLINFO_ACK_CHANNEL_RX = 0x1,
    CCCI_CELLINFO_CHANNEL_TX = 0x2,
    CCCI_CELLINFO_ACK_CHANNEL_TX = 0x3,
    CCCI_MD_SCP_RX_LB_IT = 0x4,
    CCCI_MD_SCP_TX_LB_IT = 0x5,
    CCCI_SCP_USR0_RX = 0x6,
    CCCI_SCP_USR0_TX = 0x7,
    CCCI_SCP_USR1_RX = 0x8,
    CCCI_SCP_USR1_TX = 0x9,
    CCCI_SCP_USR2_RX = 0xa,
    CCCI_SCP_USR2_TX = 0xb,
    CCCI_CH_MAX,
};

// MD3 has its own channel numbers, but should not be more than CCCI_CH_MAX
enum {
    CCCI_C2K_CH_AUDIO = 0x0,
    CCCI_C2K_CH_GEOFENCE = 0x1,
};

enum {
    IN = 0,
    OUT
};

struct ccci_ipi_msg { // should be less than [SHARE_BUF_SIZE - 16] (48bytes) in scp_ipi.h
    uint16_t md_id;
    uint16_t op_id;
    uint32_t data[1];
} __PACKED;

/*
 * peer ID:
 * AP<->MD1: 0
 * AP<->MD3: 0
 * SCP<->MD1: 1
 * SCP<->MD3: 2
 */

struct ccci_header {
    uint32_t data[2]; /* assume data[1] is data length */
    uint16_t peer_channel: 12;
    uint8_t peer_id: 4;
    uint16_t seq_num: 15;
    uint8_t assert_bit: 1;
    uint32_t reserved;
} __PACKED;

struct ccci_ipi_queue_event {
    unsigned char id;
    struct ccci_ipi_msg data;
};

struct ccci_rx_queue_event {
    unsigned char id;
};

struct ccci_ringbuf;

struct ccci_modem {
    unsigned char index;
    unsigned char state;

    unsigned char *smem_base_addr;
    struct ccci_ringbuf *smem_ring_buff[MD_QUEUE_NUM];
    short seq_nums[2][CCCI_CH_MAX];

#ifdef CCCI_SENSOR_SUPPORT
    struct SensorDescriptor_t sensor;
#endif
    unsigned char ccism_rx_msg[CCCI_MTU + sizeof(struct ccci_header)]; //scratch for sensor or loopback
};

typedef void (*CCCI_RX_FUNC_T)(void *);

struct ccci_rx_callback {
    unsigned char host_id;
    unsigned char ch_id;
    CCCI_RX_FUNC_T callback;
};

extern unsigned int ccci_log_level;
#if 1
#define CCCI_DBG_MSG(idx, tag, fmt, args...) \
do { \
    if(ccci_log_level == 1) \
        PRINTF_D("[CCCI%d/" tag "]" fmt, (idx+1), ##args); \
} while(0)
#define CCCI_INF_MSG(idx, tag, fmt, args...) PRINTF_I("[CCCI%d/" tag "]" fmt, (idx+1), ##args)
#else
#define CCCI_DBG_MSG(idx, tag, fmt, args...)
#define CCCI_INF_MSG(idx, tag, fmt, args...)
#endif
#define CCCI_ERR_MSG(idx, tag, fmt, args...) PRINTF_E("[CCCI%d/ERR/" tag "]" fmt, (idx+1), ##args)

// exported API
void ccci_init(void);
int ccci_geo_fence_send(int md_id, void *buf, int length);
int ccci_user_msg_send(int md_id, int ch_id, void *buf, int length);

int ccci_register_rx(int host_id, int ch_id, CCCI_RX_FUNC_T callback);

// internal API
int sensor_modem_notify();
void ccci_trigger_rx_task(int md_id);
int ccci_interface_init(void);
void ccci_reset_hw(int md_id);
void ccci_wakeup_md(int md_id, int qno);
void ccci_enable_irq(int md_id);
extern void mdelay(unsigned long msec);

#endif /* __CCCI_H__ */
