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

#ifndef ADSP_IPI_H
#define ADSP_IPI_H

enum ipi_id {
    IPI_WDT = 0,
    IPI_TEST1,
    IPI_LOGGER_ENABLE,
    IPI_LOGGER_WAKEUP,
    IPI_LOGGER_INIT_A,
    IPI_TRAX_ENABLE,
    IPI_TRAX_DONE,
    IPI_TRAX_INIT_A,
    IPI_VOW,
    IPI_AUDIO,
    IPI_DVT_TEST,
    IPI_TIME_SYNC,
    IPI_CONSYS,
    IPI_ADSP_A_READY,
    IPI_APCCCI,
    IPI_ADSP_A_RAM_DUMP,
    IPI_DVFS_DEBUG,
    IPI_DVFS_FIX_OPP_SET,
    IPI_DVFS_FIX_OPP_EN,
    IPI_DVFS_LIMIT_OPP_SET,
    IPI_DVFS_LIMIT_OPP_EN,
    IPI_DVFS_SUSPEND,
    IPI_DVFS_SLEEP,
    IPI_DVFS_WAKE,
    IPI_DVFS_SET_FREQ,
    IPI_CHRE,
    IPI_CHREX,
    IPI_ADSP_PLL_CTRL,
    IPI_DO_AP_MSG,
    IPI_DO_ADSP_MSG,
    IPI_MET_ADSP,
    IPI_ADSP_TIMER,
    NR_IPI,
};

#define IPI_AS_WAKEUP_SRC
#define ADSP_IPI_DEBUG  (1)

//extern unsigned char _ADSP_IPC_SHARE_BUFFER_ADDR;
//#define ADSP_IPC_SHARE_BUFFER     ((volatile unsigned int*)&_ADSP_IPC_SHARE_BUFFER_ADDR)



#define IPC_ADSP2HOST_BIT      (1 << 0)  //sys_cirq write 1 trigger
#define ADSPA_2_SPM_BIT        (1 << 0)  //ADSP2SPM wakeup write 1 trigger


#define IPC0    0 /* IPI */
#define IPC1    1 /* IPI B*/
#define IPC2    2 /* awake*/
#define IPC3    3 /* awake B*/
#define IPC_MAX  4

#define IPC0_BIT   (1 << 0)
#define IPC1_BIT   (1 << 1)
#define IPC2_BIT   (1 << 2)
#define IPC3_BIT   (1 << 3)

#define IPC_HOST2ADSP_A     IPC0_BIT

#define IPI_PRINT_THRESHOLD 1000

#define IPI_STAMP_DUMP     0
/*
 * revise for diff adsp core
 */
#define ADSP_TO_HOST_REG       ADSP_GENERAL_IRQ_SET
#define IPC_CLEAR_BIT          IPC_HOST2ADSP_A
#define IPC_HANDLER_IRQn       IPC0_IRQn
#define IPC_ADSP2SPM_BIT       ADSPA_2_SPM_BIT


enum ipi_status {
    ERROR =-1,
    DONE,
    BUSY,
};
enum ipi_dir {
    IPI_ADSP2AP = 0,
    IPI_ADSP2SPM,
    IPI_ADSP2CONN,
    IPI_ADSP2MD
};

typedef enum ipi_status ipi_status;

typedef void(*ipi_handler_t)(int id, void * data, unsigned int len);
typedef void(*ipc_handler_t)(void);

struct ipi_desc_t {
    uint32_t is_wakeup_src;
    ipi_handler_t handler;
    const char  *name;
};

struct ipc_desc_t {
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

void adsp_ipi_init(void);

ipi_status adsp_ipi_registration(enum ipi_id id, ipi_handler_t handler, const char *name);
ipi_status adsp_ipi_send(enum ipi_id id, void* buf, uint32_t len, uint32_t wait,enum ipi_dir dir);
ipi_status adsp_ipi_send_ipc(enum ipi_id id, void* buf, uint32_t len, uint32_t wait,enum ipi_dir dir);
ipi_status adsp_ipi_status(enum ipi_id id);
void adsp_ipi_wakeup_ap_registration(enum ipi_id id);
void ipi_adsp2spm(void);
void adsp_awake_init(void);
uint32_t is_ipi_busy(void);
void request_ipc(uint32_t ipc_num, ipc_handler_t handler, const char *name);
#endif
