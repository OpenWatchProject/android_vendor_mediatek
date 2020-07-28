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
#ifndef AUDIO_SPEAKER_PROTECTION_PARAMS_H
#define AUDIO_SPEAKER_PROTECTION_PARAMS_H

#include <audio_ringbuf.h>

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif


#ifndef DRV_WriteReg32
#define DRV_WriteReg32(addr,data)   ((*(volatile unsigned int *)(addr)) = (unsigned int)data)
#endif
#ifndef DRV_Reg32
#define DRV_Reg32(addr)             (*(volatile unsigned int *)(addr))
#endif
#define vSetVarBit(variable,bit) variable =   ((variable) | (1<<(bit)))
#define vResetVarBit(variable,bit) variable = ((variable) & (~(1<<(bit))))

/*
  *    param  for speaker protection
  */

#define SPK_IPI_MSG_A2D_BASE (0x200)

typedef enum {
    SPK_PROTECT_OPEN = 0x1,
    SPK_PROTECT_CLOSE,
    SPK_PROTECT_PREPARE,
    SPK_PROTECT_PLATMEMPARAM,
    SPK_PROTECT_DLMEMPARAM,
    SPK_PROTECT_IVMEMPARAM,
    SPK_PROTECT_HWPARAM,
    SPK_PROTECT_DLCOPY,
    SPK_PROTECT_START,
    SPK_PROTECT_STOP,
    SPK_PROTECT_SETPRAM,
    SPK_PROTECT_NEEDDATA,
    SPK_PROTTCT_PCMDUMP_ON,
    SPK_PROTTCT_PCMDUMP_OFF,
    SPK_PROTECT_PCMDUMP_OK,
    SPK_PROTECT_IRQDL,
    SPK_PROTECT_IRQUL,
    SPK_PROTECT_SPEECH_OPEN = 0x101,
    SPK_PROTECT_SPEECH_CLOSE,
    SPK_PROTECT_SPEECH_PREPARE,
    SPK_PROTECT_SPEECH_MDFEEDBACKPARAM,
    SPK_PROTECT_SPEECH_DLMEMPARAM,
    SPK_PROTECT_SPEECH_IVMEMPARAM,
    SPK_PROTECT_SPEECH_HWPARAM,
    SPK_PROTECT_SPEECH_DLCOPY,
    SPK_PROTECT_SPEECH_START,
    SPK_PROTECT_SPEECH_STOP,
    SPK_PROTECT_SPEECH_SETPRAM,
    SPK_PROTECT_SPEECH_NEEDDATA,
    SPK_PROTECT_SPEECH_IRQDL,
    SPK_PROTECT_SPEECH_IRQUL,
    SPK_PROTECT_CONFIGURE,
    SPK_PROTECT_CALIB,
    SPK_PROTECT_VAR,
    SPK_PROTECT_GET_TCM_BUF,

    /* volume */
    SPK_MSG_A2D_UL_GAIN = SPK_IPI_MSG_A2D_BASE + 0x00,
    SPK_MSG_A2D_DL_GAIN,

    /* device environment info */
    SPK_IPI_MSG_A2D_TASK_CFG = SPK_IPI_MSG_A2D_BASE + 0x10,
    SPK_IPI_MSG_A2D_LIB_CFG,
    SPK_IPI_MSG_A2D_PARAM,

    /* function control*/
    SPK_IPI_MSG_A2D_SPH_ON = SPK_IPI_MSG_A2D_BASE + 0x20,
    SPK_IPI_MSG_A2D_TTY_ON,

    /* speech enhancement control*/
    SPK_IPI_MSG_A2D_UL_MUTE_ON = SPK_IPI_MSG_A2D_BASE + 0x30,
    SPK_IPI_MSG_A2D_DL_MUTE_ON,
    SPK_IPI_MSG_A2D_UL_ENHANCE_ON,
    SPK_IPI_MSG_A2D_DL_ENHANCE_ON,
    SPK_IPI_MSG_A2D_BT_NREC_ON,

    /* tuning tool*/
    SPK_IPI_MSG_A2D_SET_ADDR_VALUE = SPK_IPI_MSG_A2D_BASE + 0x40,
    SPK_IPI_MSG_A2D_GET_ADDR_VALUE,
    SPK_IPI_MSG_A2D_SET_KEY_VALUE,
    SPK_IPI_MSG_A2D_GET_KEY_VALUE,


    /* debug */
    SPK_IPI_MSG_A2D_PCM_DUMP_ON = SPK_IPI_MSG_A2D_BASE + 0xA0,
    SPK_IPI_MSG_A2D_LIB_LOG_ON,
} IPI_RECEIVED_SPK_PROTECTION;


typedef enum {
    SPEAKER_PROTECTION_TASK_BITS_PER_SAMPLE_16,
    SPEAKER_PROTECTION_TASK_BITS_PER_SAMPLE_32,
} Speaker_Procetion_Task_Bits_Per_Sample_t;

typedef enum {
    SPEAKER_PROTECTION_TASK_DL_SR_8K,
    SPEAKER_PROTECTION_TASK_DL_SR_16K,
    SPEAKER_PROTECTION_TASK_DL_SR_32K,
    SPEAKER_PROTECTION_TASK_DL_SR_44K,
    SPEAKER_PROTECTION_TASK_DL_SR_48K,
    SPEAKER_PROTECTION_TASK_DL_SR_96K,
    SPEAKER_PROTECTION_TASK_DL_SR_192K,
} Speaker_Protection_DL_SAMPLERATE_t;

typedef enum {
    Speaker_protect_idle = 0x0,
    Speaker_protect_playback,
    Speaker_protect_speech,
} Speaker_Protection_Scenario;

/*
     phys_addr : physical memory of this block
     virt_addr : virtual memory of this block
     memory_type : Dram : 1 , Sram : 0, Others....
     memory_buf : buffer information of this block.
*/
typedef struct {
    uint32_t phys_addr;
    uint32_t virt_addr;
    uint32_t memory_type;
    uint32_t memory_id;
    struct RingBuf memory_buf;
} Memory_Info_t;

typedef struct {
    uint32_t mdlsamplerate;
    uint32_t mdlformat;
    uint32_t mdlchannel;
    uint32_t mdperiodsize;
    uint32_t mscenario;
    Memory_Info_t rMemPlatformDL;
    Memory_Info_t rMemSpkDL;
    Memory_Info_t rMemIVSpk;
    Memory_Info_t rMemMDDL;
} uSpeaker_Protection_Info_t;

typedef struct uSpeaker_Protection_Task_msg_t {
    uint16_t msg_id;     /* defined by user */
    uint32_t param1;     /* see audio_ipi_msg_data_t */
    uint32_t param2;     /* see audio_ipi_msg_data_t */
    uint32_t payload0;
    uint32_t payload1;
    uint32_t payload2;
    uint32_t payload3;
    uint32_t payload4;
} uSpeaker_Protection_Task_msg_t;

struct mtk_memif_reg {
    int reg_base;
    int reg_cur;
    int reg_end;
};

struct mtk_irq_info {
    int scp_irq_reg;
    int irq_mode;
    int irq_shift_bit;
};

enum spk_memif {
    SPK_MEMIF_DL,
    SPK_MEMIF_IV,
    SPK_MEMIF_MD_UL,
    SPK_MEMIF_NUM
};

int spkprotect_dma_transaction_wrap(uint32_t dst_addr, uint32_t src_addr,
                                    uint32_t len, uint8_t IsDram);

void spk_RingBuf_copyFromLinear_dma(struct RingBuf *RingBuf1, const char *buf,
                                    uint32_t count, const char bDramPlayback);

int spkprotect_dma_length_fourbyte(uint32_t len, uint32_t dst_addr,
                                   uint32_t src_addr);
void spkprotect_memcpy_dma(const char *target, const char *source, size_t count,
                           uint8_t Dramplaystate);

#endif // end of AUDIO_SPEAKER_PROTECTION_PARAMS_H

