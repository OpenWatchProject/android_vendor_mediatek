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

#ifndef SCP_IPI_H
#define SCP_IPI_H

enum ipi_id {
    IPI_WDT = 0,
    IPI_TEST1,
    IPI_LOGGER_ENABLE,
    IPI_LOGGER_WAKEUP,
    IPI_LOGGER_INIT_A,
    IPI_VOW,                    /* 5 */
    IPI_AUDIO,
    IPI_DVT_TEST,
    IPI_SENSOR,
    IPI_TIME_SYNC,
    IPI_SHF,                    /* 10 */
    IPI_CONSYS,
    IPI_SCP_A_READY,
    IPI_APCCCI,
    IPI_SCP_A_RAM_DUMP,
    IPI_DVFS_DEBUG,             /* 15 */
    IPI_DVFS_FIX_OPP_SET,
    IPI_DVFS_FIX_OPP_EN,
    IPI_DVFS_LIMIT_OPP_SET,
    IPI_DVFS_LIMIT_OPP_EN,
    IPI_DVFS_DISABLE,           /* 20 */
    IPI_DVFS_SLEEP,
    IPI_DVFS_WAKE,
    IPI_DVFS_SET_FREQ,
    IPI_CHRE,
    IPI_CHREX,                  /* 25 */
    IPI_SCP_PLL_CTRL,
    IPI_DO_AP_MSG,
    IPI_DO_SCP_MSG,
    IPI_MET_SCP,
    IPI_SCP_TIMER,              /* 30 */
    IPI_SCP_ERROR_INFO,
    IPI_SCPCTL,
    IPI_SCP_LOG_FILTER = 33,
    NR_IPI,
};

#define IPI_AS_WAKEUP_SRC
#define SCP_IPI_DEBUG  (1)

extern unsigned char _SCP_IPC_SHARE_BUFFER_ADDR;
#define SCP_IPC_SHARE_BUFFER     ((volatile unsigned int*)&_SCP_IPC_SHARE_BUFFER_ADDR)


#define SEMA_0_SCP             (1 << 0)
#define SCPA_2_SPM_BIT         (1 << 0)
#define SCPB_2_SPM_BIT         (1 << 1)

#define IPC0    0 /* IPI */
#define IPC1    1 /* IPI B*/
#define IPC2    2 /* awake*/
#define IPC3    3 /* awake B*/
#define IPC4    4 /* connsys */
#define IPC5    5
/*
#define IPC6    6
#define IPC7    7
#define IPC8    8
#define IPC9    9
#define IPC10   10
#define IPC11   11
#define IPC12   12
#define IPC13   13
#define IPC14   14
#define IPC15   15
#define IPC16   16
#define IPC17   17
*/
#define IPC_MAX   6
#define IPC0_BIT   (1 << 0)
#define IPC1_BIT   (1 << 1)
#define IPC2_BIT   (1 << 2)
#define IPC3_BIT   (1 << 3)
#define IPC4_BIT   (1 << 4)
#define IPC5_BIT   (1 << 5)
#define IPC6_BIT   (1 << 6)
#define IPC7_BIT   (1 << 7)
#define IPC8_BIT   (1 << 8)
#define IPC9_BIT   (1 << 9)
#define IPC10_BIT   (1 << 10)
#define IPC11_BIT   (1 << 11)
#define IPC12_BIT   (1 << 12)
#define IPC13_BIT   (1 << 13)
#define IPC14_BIT   (1 << 14)
#define IPC15_BIT   (1 << 15)
#define IPC16_BIT   (1 << 16)
#define IPC17_BIT   (1 << 17)


#define IPC_HOST2SCP_A     IPC0_BIT
#define IPC_HOST2SCP_B     IPC1_BIT
#define IPI_PRINT_THRESHOLD 1000

#define IPC_MD2SPM  (1<<0)

#define IPI_STAMP_DUMP     0
/*
 * revise for diff scp core
 */
#define SCP_TO_HOST_REG        SCP_A_TO_HOST_REG
#define IPC_CLEAR_BIT          IPC_HOST2SCP_A
#define IPC_HANDLER_IRQn       IPC0_IRQn
#define IPC_SCP2SPM_BIT        SCPA_2_SPM_BIT
#define AWAKE_IPC_HANDLER_IRQn       IPC2_IRQn
#define AWAKE_IPC_CLEAR_BIT          IPC2_BIT

enum ipi_status {
    ERROR =-1,
    DONE,
    BUSY,
};
enum ipi_dir {
    IPI_SCP2AP = 0,
    IPI_SCP2SPM,
    IPI_SCP2CONN,
    IPI_SCP2MD
};

typedef enum ipi_status ipi_status;

typedef void(*ipi_handler_t)(int id, void * data, unsigned int len);
typedef void(*ipc_handler_t)(void);

struct ipi_desc_t {
#ifdef CFG_IPI_STAMP_SUPPORT
#define SCP_IPI_ID_STAMP_SIZE 3
    uint64_t recv_timestamp[SCP_IPI_ID_STAMP_SIZE]; //recv_flag[recv_count] recv_timestamp[Timestamp]
    uint64_t send_timestamp[SCP_IPI_ID_STAMP_SIZE]; //send_flag[send_count] send_timestamp[Timestamp]
    uint32_t recv_flag[SCP_IPI_ID_STAMP_SIZE];
    uint32_t send_flag[SCP_IPI_ID_STAMP_SIZE];
#endif
    uint32_t recv_count;
    uint32_t init_count;
    uint32_t success_count;
    uint32_t busy_count;
    uint32_t error_count;
    uint32_t last_handled;
    uint32_t is_wakeup_src;
    ipi_handler_t handler;
    const char  *name;
};

struct ipc_desc_t {
    uint64_t last_enter;
    uint64_t last_exit;
    uint64_t max_duration;
    ipc_handler_t handler;
    const char  *name;
};

#define SHARE_BUF_SIZE 288
struct share_obj {
    enum ipi_id id;
    unsigned int len;
    unsigned char reserve[8];
    unsigned char share_buf[SHARE_BUF_SIZE - 16];
};

void scp_ipi_init(void);

ipi_status scp_ipi_registration(enum ipi_id id, ipi_handler_t handler, const char *name);
ipi_status scp_ipi_send(enum ipi_id id, void* buf, uint32_t len, uint32_t wait,enum ipi_dir dir);
ipi_status scp_ipi_status(enum ipi_id id);
void scp_ipi_wakeup_ap_registration(enum ipi_id id);
void ipi_status_dump(void);
void ipi_status_dump_id(enum ipi_id id);
void ipi_scp2spm(void);
void scp_awake_init(void);
uint32_t is_ipi_busy(void);
void request_ipc(uint32_t ipc_num, ipc_handler_t handler, const char *name);
#endif
