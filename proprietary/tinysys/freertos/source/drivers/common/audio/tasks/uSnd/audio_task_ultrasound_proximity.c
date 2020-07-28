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

#include "audio_task_ultrasound_proximity.h"

#include <interrupt.h>
#include <queue.h>
#include <wakelock.h>
#include <dvfs.h>
#include "audio_messenger_ipi.h"
#include "audio_task_factory.h"
// #include "uSnd_service.h"
#include "audio_hw_reg.h"

#define MAX_MSG_QUEUE_SIZE (8)

#define LOCAL_TASK_STACK_SIZE (1024)
#define LOCAL_TASK_NAME "uSnd"
#define LOCAL_TASK_PRIORITY (2)

#define IRQ4_SOURCE 0x10

#define UL_SAMPLE_PER_S          (96000)
// MT6799 is 32000
#if 1
   #define DL_SPH_SAMPLE_PER_S       (32000)
#else
   #define DL_SPH_SAMPLE_PER_S       (16000)
#endif

#define DL_SAMPLE_PER_S_96K       (96000)
#define DL_SAMPLE_PER_S_192K     (192000)

#define CM4_SRAM_ADDR_MAPPING(phyAdd) ((phyAdd)-AFE_INTERNAL_SRAM_PHY_BASE+AFE_INTERNAL_SRAM_CM4_MAP_BASE)

#define USND_E(fmt, args...)    PRINTF_E("[uSnd] ERR: "fmt, ##args)
#define USND_W(fmt, args...)    PRINTF_W("[uSnd] WARM: "fmt, ##args)
#define USND_L(fmt, args...)    PRINTF_I("[uSnd] LOG: "fmt, ##args)
#define USND_D(fmt, args...)    PRINTF_D("[uSnd] DBG: "fmt, ##args)

// #define ULTRASOUND_MASS_LOG
// #define ULTRASOUND_DEMO_LOOPBACK
// ============================================================================
typedef enum {
    USND_TASK_BITS_PER_SAMPLE_16,
    USND_TASK_BITS_PER_SAMPLE_32,
}uSndTask_Bits_Per_Sample_t;

typedef enum {
    USND_TASK_DL_SR_96K,
    USND_TASK_DL_SR_192K,
}uSndTask_DL_SAMPLERATE_t;

typedef struct {
    uSndTask_Bits_Per_Sample_t bitPerSample;
    uSndTask_DL_SAMPLERATE_t dlSampleRate;

    // UL
    int16_t *ulDataBuf;
    uint32_t ulBufSize;  // samples. ulBufSize = ulFrameSize * 2
    uint32_t ulFrameSize;  // samples
    uint32_t ulBufWriteIdx;

    uint32_t ulSramFrameSizeInByte;  // bytes
    uint32_t ulSramRead;
    uint32_t ulSramReadNowInIRQ; // new added

    // DL SPH
    int16_t *mddlSphDataBuf;
    uint32_t mddlSphBufSize;  // unit is samples
    uint32_t mddlSphFrameSize;  // samples
    uint32_t mddlSphBufWriteIdx;

    uint32_t mddlSphSramFrameSizeInByte;  // bytes
    uint32_t mddlSphSramRead;
    uint32_t mddlSphSramReadNowInIRQ; // new added

    uint32_t dlSramFrameSize;
    uint32_t dlSramFrameSizeInByte;
    // uint32_t dlSramRead;
    uint32_t dlSramWrite;

    uint32_t isFirstIrqCome;
    uint32_t irqTick;


    void (*ulHandler)(void *);
    void *ulHdlParam;

    void (*dlHandler)(void *);
    void *dlHdlParam;
} uSndInfo_t;

typedef struct uSndTask_msg_t {
    uint16_t msg_id;     /* defined by user */
    uint32_t param1;     /* see audio_ipi_msg_data_t */
    uint32_t param2;     /* see audio_ipi_msg_data_t */

    uint32_t payload0;
    uint32_t payload1;
    uint32_t payload2;
    uint32_t payload3;
    uint32_t payload4;
} uSndTask_msg_t;

typedef struct {
    uint16_t ipi_msgid;
    // uint32_t para1;
    // uint32_t para2;
    // uint32_t para3;
    // uint32_t para4;
}uSndServCmd_T;

typedef enum {
    USND_TASK_MSGID_ON_DONE,
    USND_TASK_MSGID_OFF_DONE,
    USND_TASK_MSGID_PROCESS,
    USND_TASK_MSGID_DLMIX,
} uSnd_task_msgid_t;

typedef enum {
    AUDIO_TASK_USND_MSG_ID_OFF = 0x0,  // VOICE_ULTRA_DISABLE_ID
    AUDIO_TASK_USND_MSG_ID_ON = 0x1,  // VOICE_ULTRA_ENABLE_ID

    AUDIO_TASK_USND_MSG_ID_IRQ = 0x4444,
}uSndTask_msg_id_t;

QueueHandle_t cmdQueue = NULL;
TaskHandle_t xUSSHandle = NULL;
static uSndInfo_t uSndInfo;
static void uSnd_serv_init(void);
static void uSnd_serv_deinit(void);
// ============================================================================
#ifdef WAKELOCK
wakelock_t ultrasound_proximity_task_monitor_wakelock;
#endif
// #define COUNT_TIME_DIFF
#ifdef COUNT_TIME_DIFF
unsigned int start_time;

volatile unsigned int *uSnd_ITM_CONTROL = (unsigned int *)0xE0000E80;
volatile unsigned int *uSnd_DWT_CONTROL = (unsigned int *)0xE0001000;
volatile unsigned int *uSnd_DWT_CYCCNT = (unsigned int *)0xE0001004;
volatile unsigned int *uSnd_DEMCR = (unsigned int *)0xE000EDFC;

#define CPU_RESET_CYCLECOUNTER() \
do { \
 *uSnd_DEMCR = *uSnd_DEMCR | 0x01000000; \
 *uSnd_DWT_CYCCNT = 0; \
 *uSnd_DWT_CONTROL = *uSnd_DWT_CONTROL | 1 ; \
 } while (0)

// Test Method
// CPU_RESET_CYCLECOUNTER();
// start_time = *uSnd_DWT_CYCCNT;
#endif

void task_ultrasound_setDlHandler(void (*dlHandler)(void *), void *dlHdlParam) {
    uSndInfo.dlHandler = dlHandler;
    uSndInfo.dlHdlParam = dlHdlParam;
}

void task_ultrasound_setUlHandler(void (*ulHandler)(void *), void *ulHdlParam) {
    uSndInfo.ulHandler = ulHandler;
    uSndInfo.ulHdlParam = ulHdlParam;
}

uint32_t task_ultrasound_getMddlSphSampleNumberPerFrame(void) {
    AudioTask *task = get_task_by_scene(TASK_SCENE_VOICE_ULTRASOUND);

    if ( AUDIO_TASK_INIT != task->state && AUDIO_TASK_WORKING != task->state ) {
        AUD_LOG_E("%s(), get DlSph without task running!", __FUNCTION__);
        return 0;
    }

    return uSndInfo.mddlSphFrameSize;
}

int16_t *task_ultrasound_getMddlSphData(void) {
    AudioTask *task = get_task_by_scene(TASK_SCENE_VOICE_ULTRASOUND);

    if ( AUDIO_TASK_INIT != task->state && AUDIO_TASK_WORKING != task->state ) {
        AUD_LOG_E("%s(), get DlSph without task running!", __FUNCTION__);
        return 0;
    }

    return uSndInfo.mddlSphDataBuf;
}

uint32_t task_ultrasound_getDlSampleNumberPerFrame(void) {
    AudioTask *task = get_task_by_scene(TASK_SCENE_VOICE_ULTRASOUND);

    if ( AUDIO_TASK_INIT != task->state && AUDIO_TASK_WORKING != task->state ) {
        AUD_LOG_E("%s(), write DL data without task running!", __FUNCTION__);
        return 0;
    }

    return uSndInfo.dlSramFrameSize;
}

/**
	@dlData: PCM data array
	@len: number of samples. for 96k, should be 960.
*/
void task_ultrasound_writeDlData(int16_t *dlData, uint32_t len) {
    uint32_t lenInByte;
    uint32_t sramBase;
    uint32_t sramEnd;
    // uint32_t sramCur;

    AudioTask *task = get_task_by_scene(TASK_SCENE_VOICE_ULTRASOUND);
    if ( AUDIO_TASK_INIT != task->state && AUDIO_TASK_WORKING != task->state ) {
        AUD_LOG_E("%s(), write DL data without task running!\n", __func__);
        return;
    }

    sramBase = drv_reg32(AFE_DL3_BASE);
    sramEnd = drv_reg32(AFE_DL3_END)+1;
    // sramCur = drv_reg32(AFE_DL3_CUR);

    if (0 == sramBase || 1 == sramEnd) {
        AUD_LOG_W("%s(), return due to no hardware setting!\n", __func__);
        return;
    }
#ifdef ULTRASOUND_MASS_LOG
    AUD_LOG_D("uSndInfo.dlSramWrite=0x%x, DL3 sramBase=0x%x, sramEnd=0x%x, dlData=0x%x\n",
        uSndInfo.dlSramWrite, sramBase, sramEnd, dlData);
#endif
    if ( USND_TASK_BITS_PER_SAMPLE_16 == uSndInfo.bitPerSample ) {
        uint32_t i;
        lenInByte = len*sizeof(int16_t);
        // AUD_LOG_D("%s(), len = 0x%x, lenInByte= 0x%x, dlSramFrameSizeInByte = 0x%x\n", __func__, len, lenInByte,
            // uSndInfo.dlSramFrameSizeInByte);
        AUD_ASSERT(lenInByte == uSndInfo.dlSramFrameSizeInByte);

        for ( i = 0; i < len; i++ ) {
            *((int16_t *)(CM4_SRAM_ADDR_MAPPING(uSndInfo.dlSramWrite))) = dlData[i];

            uSndInfo.dlSramWrite = uSndInfo.dlSramWrite + sizeof(int16_t);
            if ( uSndInfo.dlSramWrite >= sramEnd ) {
                uSndInfo.dlSramWrite = uSndInfo.dlSramWrite-sramEnd+sramBase;
            }
        }

    } else {  // 24bit sample
        uint32_t i;
        AUD_ASSERT(len*sizeof(uint32_t) == uSndInfo.dlSramFrameSizeInByte);

        for ( i = 0; i < len; i++ ) {
            *((uint32_t *)(CM4_SRAM_ADDR_MAPPING(uSndInfo.dlSramWrite))) = ((((uint32_t)(dlData[i])) << 8)&0x00ffffff);

            uSndInfo.dlSramWrite = uSndInfo.dlSramWrite + sizeof(uint32_t);
            if (uSndInfo.dlSramWrite >= sramEnd) {
                uSndInfo.dlSramWrite = uSndInfo.dlSramWrite-sramEnd+sramBase;
            }
        }
    }

    // AUD_LOG_D("write done!! dlCur=0x%x, uSndInfo.dlSramWrite=0x%x", (drv_reg32(AFE_DL3_CUR)), uSndInfo.dlSramWrite);
}

uint32_t task_ultrasound_getUlSampleNumberPerFrame(void) {
    AudioTask *task = get_task_by_scene(TASK_SCENE_VOICE_ULTRASOUND);

    if ( AUDIO_TASK_INIT != task->state && AUDIO_TASK_WORKING != task->state ) {
        AUD_LOG_E("%s(), get DlSph without task running!", __FUNCTION__);
        return 0;
    }

    return uSndInfo.ulFrameSize;
}

uint32_t task_ultrasound_getUlSampleNumber(void) {
    AudioTask *task = get_task_by_scene(TASK_SCENE_VOICE_ULTRASOUND);

    if ( AUDIO_TASK_INIT != task->state && AUDIO_TASK_WORKING != task->state ) {
        AUD_LOG_E("%s(), get DlSph without task running!", __FUNCTION__);
        return 0;
    }

    return uSndInfo.ulBufSize;
}

int16_t *task_ultrasound_getUlData() {
    AudioTask *task = get_task_by_scene(TASK_SCENE_VOICE_ULTRASOUND);

    if ( AUDIO_TASK_INIT != task->state && AUDIO_TASK_WORKING != task->state ) {
        AUD_LOG_E("%s(), get DlSph without task running!", __FUNCTION__);
        return 0;
    }

    return uSndInfo.ulDataBuf;
}

// ============================================================================

static void task_ultrasound_proximity_start(AudioTask *this,
    uSndTask_Bits_Per_Sample_t bitsPerSample,
    uSndTask_DL_SAMPLERATE_t dlSampleRate) {
    uint32_t irqSr;
    uint32_t irqCn;

    if (AUDIO_TASK_IDLE != this->state) {
        AUD_LOG_E("%s Unexpected start with status=%d, %d, %d\n", __FUNCTION__, this->state);
        return;
    }

    AUD_LOG_D("%s() ===== %d, %d\n", __FUNCTION__, bitsPerSample, dlSampleRate);

    irqSr = ((drv_reg32(AFE_IRQ_MCU_CON1))>>16)&0xF;
    irqCn = drv_reg32(AFE_IRQ_MCU_CNT4);
    switch (irqSr) {
        case 0: irqSr = 8000; break;
        case 1: irqSr = 11025; break;
        case 2: irqSr = 12000; break;

        case 4: irqSr = 16000; break;
        case 5: irqSr = 22050; break;
        case 6: irqSr = 24000; break;
        case 7: irqSr = 130000; break;
        case 8: irqSr = 32000; break;
        case 9: irqSr = 44100; break;
        case 10: irqSr = 48000; break;
        case 11: irqSr = 88200; break;
        case 12: irqSr = 96000; break;
        case 13: irqSr = 176400; break;
        case 14: irqSr = 192000; break;
        case 15: irqSr = 260000; break;
    }
    uSndInfo.ulFrameSize = UL_SAMPLE_PER_S*irqCn/irqSr;
    uSndInfo.ulBufSize = (uSndInfo.ulFrameSize)*2;

    uSndInfo.ulDataBuf = kal_pvPortMalloc(uSndInfo.ulBufSize*sizeof(int16_t));
    uSndInfo.ulBufWriteIdx = 0;


    // uSndInfo.ulSramRead = drv_reg32(AFE_DAI2_BASE);


    if (uSndInfo.ulDataBuf == NULL) {
        AUD_LOG_E("%s(), kal_pvPortMalloc ulBufSize fail with size: %d\n", __func__, uSndInfo.ulBufSize);

        // reset
        uSndInfo.ulBufSize = 0;
        uSndInfo.ulFrameSize = 0;
        return;
    }

    uSndInfo.mddlSphFrameSize = DL_SPH_SAMPLE_PER_S*irqCn/irqSr;
    AUD_LOG_D("%s() ===== dlSphFraSize %d, ulFraSize %d, irqCn %d, irqSr %d\n", __FUNCTION__,
        uSndInfo.mddlSphFrameSize, uSndInfo.ulFrameSize, irqCn, irqSr);
    uSndInfo.mddlSphBufSize = (uSndInfo.mddlSphFrameSize);
    uSndInfo.mddlSphDataBuf = kal_pvPortMalloc(uSndInfo.mddlSphBufSize*sizeof(int16_t));
    uSndInfo.mddlSphBufWriteIdx = 0;

    if (uSndInfo.mddlSphDataBuf == NULL) {
        AUD_LOG_E("%s(), kal_pvPortMalloc mddlSphDataBuf fail with size: %d", __func__, uSndInfo.mddlSphBufSize);

        // reset
        uSndInfo.mddlSphBufSize = 0;
        uSndInfo.mddlSphFrameSize = 0;
        return;
    }

    uSndInfo.bitPerSample = bitsPerSample;
    uSndInfo.dlSampleRate = dlSampleRate;
    switch (bitsPerSample) {
        case USND_TASK_BITS_PER_SAMPLE_16:

            uSndInfo.ulSramFrameSizeInByte = (uSndInfo.ulFrameSize)*sizeof(int16_t);
            uSndInfo.mddlSphSramFrameSizeInByte = uSndInfo.mddlSphFrameSize*sizeof(int16_t);

            if (USND_TASK_DL_SR_96K == dlSampleRate) {
                uSndInfo.dlSramFrameSize = DL_SAMPLE_PER_S_96K*irqCn/irqSr;
                uSndInfo.dlSramFrameSizeInByte = uSndInfo.dlSramFrameSize*sizeof(int16_t);
            } else {  // USND_TASK_DL_SR_192K
                uSndInfo.dlSramFrameSize = DL_SAMPLE_PER_S_192K*irqCn/irqSr;
                uSndInfo.dlSramFrameSizeInByte = uSndInfo.dlSramFrameSize*sizeof(int16_t);
            }

            break;

        case USND_TASK_BITS_PER_SAMPLE_32:

            uSndInfo.ulSramFrameSizeInByte = (uSndInfo.ulFrameSize)*sizeof(uint32_t);
            uSndInfo.mddlSphSramFrameSizeInByte = uSndInfo.mddlSphFrameSize*sizeof(uint32_t);

            if (USND_TASK_DL_SR_96K == dlSampleRate) {
                uSndInfo.dlSramFrameSize = DL_SAMPLE_PER_S_96K*irqCn/irqSr;
                uSndInfo.dlSramFrameSizeInByte = uSndInfo.dlSramFrameSize*sizeof(uint32_t);
            } else {  // USND_TASK_DL_SR_192K
                uSndInfo.dlSramFrameSize = DL_SAMPLE_PER_S_192K*irqCn/irqSr;
                uSndInfo.dlSramFrameSizeInByte = uSndInfo.dlSramFrameSize*sizeof(uint32_t);
            }

            break;
        default:
            AUD_ASSERT(0);
    }
    AUD_LOG_D("%s(), ulDataBuf 0x%x, end 0x%x DAI2 base 0x%x, end 0x%x\n", __FUNCTION__, uSndInfo.ulDataBuf,
        uSndInfo.ulDataBuf + uSndInfo.ulBufSize, drv_reg32(AFE_DAI2_BASE), drv_reg32(AFE_DAI2_END));
    AUD_LOG_D("%s(), AWB base 0x%x, end 0x%x\n", __FUNCTION__, drv_reg32(AFE_AWB_BASE), drv_reg32(AFE_AWB_END));
    uSndInfo.isFirstIrqCome = 0;
    uSndInfo.irqTick = 0;
#ifdef WAKELOCK
    wake_lock_init(&ultrasound_proximity_task_monitor_wakelock, "ultsw");
    wake_lock(&ultrasound_proximity_task_monitor_wakelock);
#endif
    scp_wakeup_src_setup(AUDIO_WAKE_CLK_CTRL, true);
    this->state = AUDIO_TASK_INIT;

    #if 1
    {
        uSndServCmd_T ackCmd;
        ackCmd.ipi_msgid = USND_TASK_MSGID_ON_DONE;
        if ( pdPASS != xQueueSend(cmdQueue, &ackCmd, 0) ) {
            AUD_LOG_E("%s(), send fail with ipi_msgid=%d\n", ackCmd.ipi_msgid);
        };
    }
    #endif
    DRV_SetReg32(AFE_DAC_CON0, 0x10000060);  // trun on mem interface; DAI2:bit28, AWB: bit6, DL3: bit5
    AUD_LOG_D("-%s()\n", __FUNCTION__);
#ifdef COUNT_TIME_DIFF
    CPU_RESET_CYCLECOUNTER();
    start_time = *uSnd_DWT_CYCCNT;
#endif
}

static void task_ultrasound_proximity_stop(AudioTask *this) {
    if ( AUDIO_TASK_IDLE == this->state ) {
        return;
    }

    this->state = AUDIO_TASK_DEINIT;

    DRV_ClrReg32(AFE_DAC_CON0, 0x10000060);  // trun off mem interface; DAI2:bit28, AWB: bit6, DL3: bit5);

    vPortFree(uSndInfo.ulDataBuf);
    vPortFree(uSndInfo.mddlSphDataBuf);
    scp_wakeup_src_setup(AUDIO_WAKE_CLK_CTRL, false);
#ifdef WAKELOCK
    wake_unlock(&ultrasound_proximity_task_monitor_wakelock);
    wake_lock_deinit(&ultrasound_proximity_task_monitor_wakelock);
#endif
    this->state = AUDIO_TASK_IDLE;

    #if 1
    {
        uSndServCmd_T ackCmd;
        ackCmd.ipi_msgid = USND_TASK_MSGID_OFF_DONE;
        if ( pdPASS != xQueueSend(cmdQueue, &ackCmd, 0) ) {
            AUD_LOG_E("%s(), senf fail with ipi_msgid=%d\n", ackCmd.ipi_msgid);
        }
    }
    #endif
}

static void task_ultrasound_proximity_ulProc(void) {
    uint32_t i;
    uint32_t ulReadNow = uSndInfo.ulSramReadNowInIRQ;
    uint32_t ulSramEnd = drv_reg32(AFE_DAI2_END);
    uint32_t bytesRead;
    uint32_t samplesRead;

    // expect data length is one frame
    if (ulReadNow > uSndInfo.ulSramRead) {
        bytesRead = ulReadNow - uSndInfo.ulSramRead;
#ifdef ULTRASOUND_MASS_LOG
        AUD_LOG_D("1, ulReadNow=0x%x, uSndInfo.ulSramRead=0x%x, len=0x%x\n",
         ulReadNow, uSndInfo.ulSramRead, ulReadNow-uSndInfo.ulSramRead);
#endif
        //AUD_ASSERT(bytesRead >= uSndInfo.ulSramFrameSizeInByte);
    } else {
        bytesRead = ulReadNow-drv_reg32(AFE_DAI2_BASE)+(ulSramEnd+1)-uSndInfo.ulSramRead;
#ifdef ULTRASOUND_MASS_LOG
        AUD_LOG_D("2, ulReadNow=0x%x, uSndInfo.ulSramRead=0x%x, len=0x%x\n", ulReadNow, uSndInfo.ulSramRead, bytesRead);
#endif
        //AUD_ASSERT(bytesRead >= uSndInfo.ulSramFrameSizeInByte);
        // (end-write) + (read-begin) >= frame
    }
    if( bytesRead < uSndInfo.ulSramFrameSizeInByte )
    {
        PRINTF_E("Incomplete ul frame. Length=%d, expected %d in tick=%d\n", bytesRead, uSndInfo.ulSramFrameSizeInByte, uSndInfo.irqTick);
    }

    // process data to buffer
    ulReadNow = uSndInfo.ulSramRead;
    samplesRead = bytesRead >> ( (USND_TASK_BITS_PER_SAMPLE_32 == uSndInfo.bitPerSample) ? 2 : 1 );
    if (samplesRead > uSndInfo.ulFrameSize) {
        samplesRead = uSndInfo.ulFrameSize;
    }
    for ( i = 0; i < samplesRead; i++ ) {
        if (USND_TASK_BITS_PER_SAMPLE_32 == uSndInfo.bitPerSample) {
            uint32_t temp32Pmc = *(uint32_t *)(ulReadNow - AFE_INTERNAL_SRAM_PHY_BASE + AFE_INTERNAL_SRAM_CM4_MAP_BASE);
            uSndInfo.ulDataBuf[uSndInfo.ulBufWriteIdx] = (int16_t) ((temp32Pmc >> 6) & 0xffff);
            ulReadNow += (sizeof(uint32_t));
        } else {
            uint32_t temp16Pmc = *(uint16_t *)(ulReadNow - AFE_INTERNAL_SRAM_PHY_BASE + AFE_INTERNAL_SRAM_CM4_MAP_BASE);
            uSndInfo.ulDataBuf[uSndInfo.ulBufWriteIdx] = (int16_t) ((temp16Pmc&0x3fff) << 2);
            ulReadNow += (sizeof(int16_t));
        }

        if (ulReadNow > ulSramEnd) {
            ulReadNow = drv_reg32(AFE_DAI2_BASE);  // ulReadNow - ulSramEnd - 1 + drv_reg32(AFE_DAI2_BASE);
        }

        uSndInfo.ulBufWriteIdx++;
        if ((uSndInfo.ulBufWriteIdx) >= uSndInfo.ulBufSize) {
            uSndInfo.ulBufWriteIdx = uSndInfo.ulBufWriteIdx - uSndInfo.ulBufSize;
        }
    }
    // Fill missing samples with zeros
    for ( ; i < uSndInfo.ulFrameSize; i++ ) {
        uSndInfo.ulDataBuf[uSndInfo.ulBufWriteIdx] = 0;
        uSndInfo.ulBufWriteIdx++;
        if ((uSndInfo.ulBufWriteIdx) >= uSndInfo.ulBufSize) {
            uSndInfo.ulBufWriteIdx = uSndInfo.ulBufWriteIdx - uSndInfo.ulBufSize;
        }
    }

    uSndInfo.ulSramRead = ulReadNow;

    if (NULL != uSndInfo.ulHandler) {
        uSndInfo.ulHandler(uSndInfo.ulHdlParam);
    }
}


static void task_ultrasound_proximity_dlSphProc(void) {
    uint32_t i;

    uint32_t dlSphReadNow = uSndInfo.mddlSphSramReadNowInIRQ;
    uint32_t dlSphSramEnd = drv_reg32(AFE_AWB_END);
    uint32_t bytesRead;
    uint32_t samplesRead;

    if (dlSphReadNow > uSndInfo.mddlSphSramRead) {
        bytesRead = dlSphReadNow-uSndInfo.mddlSphSramRead;
#ifdef ULTRASOUND_MASS_LOG
        AUD_LOG_D("1, dlSphReadNow=0x%x, uSndInfo.mddlSphSramRead=0x%x, len=0x%x\n",
            dlSphReadNow, uSndInfo.mddlSphSramRead, dlSphReadNow-uSndInfo.mddlSphSramRead);
#endif
        //AUD_ASSERT(bytesRead >= uSndInfo.mddlSphSramFrameSizeInByte);
    } else {
        bytesRead = dlSphReadNow-drv_reg32(AFE_AWB_BASE)+(dlSphSramEnd+1)-uSndInfo.mddlSphSramRead;
#ifdef ULTRASOUND_MASS_LOG
        AUD_LOG_D("2, dlSphReadNow=0x%x, uSndInfo.mddlSphSramRead=0x%x, len=0x%x\n",
            dlSphReadNow, uSndInfo.mddlSphSramRead, bytesRead);
#endif
        //AUD_ASSERT(bytesRead >= uSndInfo.mddlSphSramFrameSizeInByte);
        // (end-write) + (read-begin) >= frame
    }
    if ( bytesRead < uSndInfo.mddlSphSramFrameSizeInByte ){
        PRINTF_E("Incomplete dlSph frame. Length=%d, expected %d in tick=%d\n", bytesRead, uSndInfo.mddlSphSramFrameSizeInByte, uSndInfo.irqTick );
    }

    // process data to buffer
    dlSphReadNow = uSndInfo.mddlSphSramRead;
    samplesRead = bytesRead >> ( (USND_TASK_BITS_PER_SAMPLE_32 == uSndInfo.bitPerSample) ? 2 : 1 );
    if (samplesRead > uSndInfo.mddlSphFrameSize) {
        samplesRead = uSndInfo.mddlSphFrameSize;
    }

    for ( i = 0; i < samplesRead; i++ ) {
        if ( USND_TASK_BITS_PER_SAMPLE_32 == uSndInfo.bitPerSample ) {  // 4 bytes per sample
            uint32_t temp32Pmc = *(uint32_t *)(dlSphReadNow - AFE_INTERNAL_SRAM_PHY_BASE +
                AFE_INTERNAL_SRAM_CM4_MAP_BASE);
            uSndInfo.mddlSphDataBuf[uSndInfo.mddlSphBufWriteIdx] = (int16_t) ((temp32Pmc >> 8) & 0xffff);
            dlSphReadNow += (sizeof(uint32_t));  // 4 bytes per sample
        } else {
            uSndInfo.mddlSphDataBuf[uSndInfo.mddlSphBufWriteIdx]
                = *(int16_t *)(dlSphReadNow - AFE_INTERNAL_SRAM_PHY_BASE + AFE_INTERNAL_SRAM_CM4_MAP_BASE);
            dlSphReadNow += (sizeof(int16_t));  // 2 bytes per sample
        }


        if (dlSphReadNow > dlSphSramEnd) {
            dlSphReadNow = drv_reg32(AFE_AWB_BASE);  // dlSphReadNow - dlSphSramEnd - 1 + drv_reg32(AFE_AWB_BASE);
        }

        uSndInfo.mddlSphBufWriteIdx++;
        if ((uSndInfo.mddlSphBufWriteIdx) >= uSndInfo.mddlSphBufSize) {
            uSndInfo.mddlSphBufWriteIdx = uSndInfo.mddlSphBufWriteIdx - uSndInfo.mddlSphBufSize;
        }
    }
    // Fill missing samples with zeros
    for ( ; i < uSndInfo.mddlSphFrameSize; i++ ) {
        uSndInfo.mddlSphDataBuf[uSndInfo.mddlSphBufWriteIdx] = 0;

        uSndInfo.mddlSphBufWriteIdx++;
        if ((uSndInfo.mddlSphBufWriteIdx) >= uSndInfo.mddlSphBufSize) {
            uSndInfo.mddlSphBufWriteIdx = uSndInfo.mddlSphBufWriteIdx - uSndInfo.mddlSphBufSize;
        }
    }

    // AUD_LOG_D("update mddlSphSramRead 0x%x\n",dlSphReadNow);
    uSndInfo.mddlSphSramRead = dlSphReadNow;
}


static void task_ultrasound_proximity_dlProc(void) {
    if (NULL != uSndInfo.dlHandler) {
        uSndInfo.dlHandler(uSndInfo.dlHdlParam);
    }
}

/**
	The task like HISR.
	Please do data moving inside the task.
*/
static void task_ultrasound_proximity_task_loop(void *pvParameters) {
    AudioTask *this = (AudioTask *)pvParameters;
    uSndTask_msg_t uSndMsg;

    while (1) {
        xQueueReceive(this->msg_idx_queue, &uSndMsg, portMAX_DELAY);

        switch (uSndMsg.msg_id) {
            case AUDIO_TASK_USND_MSG_ID_ON:
                AUD_LOG_D("%s(), uSndMsg=0x%x, 0x%x, 0x%x, %d, %d, %d, %d, %d\n",
                            __func__, uSndMsg.msg_id, uSndMsg.param1, uSndMsg.param2,
                            uSndMsg.payload0, uSndMsg.payload1, uSndMsg.payload2, uSndMsg.payload3, uSndMsg.payload4);

                AUD_ASSERT(DL_SPH_SAMPLE_PER_S == uSndMsg.payload0);
                AUD_ASSERT(UL_SAMPLE_PER_S == uSndMsg.payload2);

                if ( 96000 == uSndMsg.payload1 ) {
                    if ( 2 == uSndMsg.payload3 ) {
                        task_ultrasound_proximity_start(this, USND_TASK_BITS_PER_SAMPLE_16, USND_TASK_DL_SR_96K);
                    } else if ( 4 == uSndMsg.payload3 ) {
                        task_ultrasound_proximity_start(this, USND_TASK_BITS_PER_SAMPLE_32, USND_TASK_DL_SR_96K);
                    } else {
                        AUD_ASSERT(0);
                    }
                } else if ( 192000 == uSndMsg.payload1 ) {
                    if ( 2 == uSndMsg.payload3 ) {
                        task_ultrasound_proximity_start(this, USND_TASK_BITS_PER_SAMPLE_16, USND_TASK_DL_SR_192K);
                    } else if ( 4 == uSndMsg.payload3 ) {
                        task_ultrasound_proximity_start(this, USND_TASK_BITS_PER_SAMPLE_32, USND_TASK_DL_SR_192K);
                    } else {
                        AUD_ASSERT(0);
                    }
                } else {
                    AUD_ASSERT(0);  // unexpect downlink sampling rate
                }

                break;
            case AUDIO_TASK_USND_MSG_ID_OFF:
                task_ultrasound_proximity_stop(this);
                break;
            case AUDIO_TASK_USND_MSG_ID_IRQ:
                {
#ifdef COUNT_TIME_DIFF
                uint32_t end_time;
                end_time = *uSnd_DWT_CYCCNT;
                AUD_LOG_D("%s(), end_time-start_time = %d\n", __FUNCTION__, (end_time-start_time));

                // CPU_RESET_CYCLECOUNTER();
                start_time = end_time;  // *uSnd_DWT_CYCCNT;
#endif
                if (AUDIO_TASK_INIT == this->state) {
                    //uSndInfo.ulSramRead = drv_reg32(AFE_DAI2_CUR);
                    //uSndInfo.mddlSphSramRead = drv_reg32(AFE_AWB_CUR);

                    uSndInfo.dlSramWrite = drv_reg32(AFE_DL3_CUR)+((uSndInfo.dlSramFrameSizeInByte*3)>>1);  // write ahead
                    if (uSndInfo.dlSramWrite > drv_reg32(AFE_DL3_END)) {
                        uSndInfo.dlSramWrite = uSndInfo.dlSramWrite-drv_reg32(AFE_DL3_END)-1+drv_reg32(AFE_DL3_BASE);
                    }

                    // AUD_LOG_D(
                    // "ulSramRead=0x%x, 0x%x, mddlSphSramRead=0x%x, mddlSphSramFrameSizeInByte=0x%x,
                    //    dlSramWrite=0x%x\n", uSndInfo.ulSramRead, uSndInfo.ulSramFrameSizeInByte,
                    //    uSndInfo.mddlSphSramRead, uSndInfo.mddlSphSramFrameSizeInByte, uSndInfo.dlSramWrite);

                    if ( uSndInfo.isFirstIrqCome > 0 ) {
                        this->state = AUDIO_TASK_WORKING;
                    }
                } else if (AUDIO_TASK_WORKING == this->state) {
                        // AUD_LOG_D("AUDIO_TASK_WORKING\n");
                        task_ultrasound_proximity_ulProc();
                        task_ultrasound_proximity_dlSphProc();
                        task_ultrasound_proximity_dlProc();
#ifdef ULTRASOUND_MASS_LOG
                        AUD_LOG_D("AUDIO_TASK_WORKING done\n");
#endif
                }
                }
                break;
            default:
                AUD_LOG_I("unknown uSndMsg=0x%x, 0x%x, 0x%x, 0x%x\n", uSndMsg.msg_id, uSndMsg.param1, uSndMsg.param2);
        }
    }
}


// ============================================================================

static void task_ultrasound_proximity_constructor(struct AudioTask *this) {
    AUD_ASSERT(this != NULL);

    /* assign initial value for class members & alloc private memory here */
    this->scene = TASK_SCENE_VOICE_ULTRASOUND;
    this->state = AUDIO_TASK_IDLE;

    /* queue */
    this->queue_idx = 0;

    this->msg_idx_queue = xQueueCreate(MAX_MSG_QUEUE_SIZE, sizeof(uSndTask_msg_t));
    AUD_ASSERT(this->msg_idx_queue != NULL);
    cmdQueue = xQueueCreate(MAX_MSG_QUEUE_SIZE, sizeof(uSndServCmd_T));
    AUD_ASSERT(cmdQueue != NULL);
}

static void task_ultrasound_proximity_destructor(struct AudioTask *this) {
    AUD_LOG_D("%s(), task_scene = %d\n", __func__, this->scene);

    /* dealloc private memory & dealloc object here */
    AUD_ASSERT(this != NULL);
    if (this->msg_idx_queue != NULL) {
        vQueueDelete(this->msg_idx_queue);
    }
    if (cmdQueue != NULL) {
        vQueueDelete(cmdQueue);
        cmdQueue = NULL;
    }
}

static void task_ultrasound_proximity_create_task_loop(struct AudioTask *this) {
    /* Note: you can also bypass this function,
             and do kal_xTaskCreate until you really need it.
             Ex: create task after you do get the enable message. */

    BaseType_t xReturn = pdFAIL;

    xReturn = kal_xTaskCreate(
                  task_ultrasound_proximity_task_loop,
                  LOCAL_TASK_NAME,
                  LOCAL_TASK_STACK_SIZE,
                  (void *)this,
                  LOCAL_TASK_PRIORITY,
                  &this->freertos_task);

    AUD_ASSERT(xReturn == pdPASS);
    uSnd_serv_init();
}

static void task_ultrasound_proximity_destroy_task_loop(struct AudioTask *this)
{
#if (INCLUDE_vTaskDelete == 1)
    if (this->freertos_task) {
        vTaskDelete(this->freertos_task);
    }
#endif
    uSnd_serv_deinit();
}

static status_t task_ultrasound_proximity_recv_message(
    struct AudioTask *this,
    struct ipi_msg_t *ipi_msg) {
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    uSndTask_msg_t uSndMsg;
    uint32_t *tempPayload = (uint32_t *)(ipi_msg->payload);

    uSndMsg.msg_id = ipi_msg->msg_id;
    uSndMsg.param1 = ipi_msg->param1;
    uSndMsg.param2 = ipi_msg->param2;
    uSndMsg.payload0 = *tempPayload;
    uSndMsg.payload1 = *(tempPayload+1);
    uSndMsg.payload2 = *(tempPayload+2);
    uSndMsg.payload3 = *(tempPayload+3);
    uSndMsg.payload4 = *(tempPayload+4);

    xQueueSendToBackFromISR(
        this->msg_idx_queue,
        &uSndMsg,
        &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    return NO_ERROR;
}

static void task_ultrasound_proximity_irq_hanlder(
    struct AudioTask *this,
    uint32_t irq_type) {
    if (irq_type == AUDIO_IRQn) {
        static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

        uSndTask_msg_t uSndMsg;
        int IRQSource;

        IRQSource = DRV_Reg32(AFE_IRQ_MCU_STATUS);
#if 0
        if ( (IRQSource & 0xff) == 0 ) {
            data = DRV_Reg32(AFE_IRQ_MCU_CLR);
            DRV_WriteReg32(AFE_IRQ_MCU_CLR,
                           data); /*Clears the MCU IRQ for AFE while all IRQ statuses are 0*/
        }
#endif
        // check IRQ 4
        // AUD_LOG_D("%s(+) %d %d\n", __func__, irq_type, IRQSource);
        if ( IRQSource & IRQ4_SOURCE ) {
            DRV_WriteReg32(AFE_IRQ_MCU_CLR, (IRQSource & AFE_IRQ_MASK));

            uSndMsg.msg_id = AUDIO_TASK_USND_MSG_ID_IRQ;

            // portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            // AUD_LOG_D("%d\n", this->state);
            if ( AUDIO_TASK_INIT == this->state || AUDIO_TASK_WORKING == this->state ) {
                if ( AUDIO_TASK_INIT == this->state ) {
                    uSndInfo.ulSramRead = drv_reg32(AFE_DAI2_CUR);
                    uSndInfo.mddlSphSramRead = drv_reg32(AFE_AWB_CUR);
                    uSndInfo.isFirstIrqCome++;
                }
                uSndInfo.ulSramReadNowInIRQ = drv_reg32(AFE_DAI2_CUR);
                uSndInfo.mddlSphSramReadNowInIRQ = drv_reg32(AFE_AWB_CUR);
                uSndInfo.irqTick++;
                // send message to task
                if (xQueueSendFromISR(this->msg_idx_queue, &uSndMsg,
                                      &xHigherPriorityTaskWoken) != pdTRUE) {
                    AUD_LOG_D("sent fail\n");
                    return;
                }
#ifdef ULTRASOUND_MASS_LOG
                AUD_LOG_D("sent1\n");
#endif
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        }
    }
}


static void task_ultrasound_proximity_hal_reboot_cbk(struct AudioTask *this, const uint8_t hal_type) {
    AUD_LOG_D("%s(), scene %d\n", __func__, this->scene);
}


AudioTask *task_ultrasound_proximity_new() {
    /* alloc object here */
    AudioTask *task = (AudioTask *)kal_pvPortMalloc(sizeof(AudioTask));
    if (task == NULL) {
        AUD_LOG_E("%s(), kal_pvPortMalloc fail!!\n", __func__);
        return NULL;
    }
    memset(task, 0, sizeof(AudioTask));
    /* only assign methods, but not class members here */
    task->constructor       = task_ultrasound_proximity_constructor;
    task->destructor        = task_ultrasound_proximity_destructor;

    task->create_task_loop  = task_ultrasound_proximity_create_task_loop;
    task->destroy_task_loop = task_ultrasound_proximity_destroy_task_loop;

    task->recv_message      = task_ultrasound_proximity_recv_message;

    task->irq_hanlder       = task_ultrasound_proximity_irq_hanlder;

    task->hal_reboot_cbk    = task_ultrasound_proximity_hal_reboot_cbk;


    return task;
}

void task_ultrasound_proximity_delete(AudioTask *task) {
    AUD_LOG_D("%s(+)\n", __func__);

    if (task == NULL) {
        AUD_LOG_E("%s(), task is NULL!!\n", __func__);
        return;
    }

    vPortFree(task);

    AUD_LOG_D("%s(-)\n", __func__);
}


/***********************************************************************************
** uSnd_serv_task
************************************************************************************/

#if 1
typedef struct {
    int16_t *playbackData;
    uint32_t playbackDataLen;  // sample
    uint32_t srcBufIdx;        // sample
} uSnd_serv_proc_t;

uSnd_serv_proc_t uSndProc;
#endif

static void uSnd_serv_task(void *pvParameters) {
    uSndServCmd_T ipiCmdInfo;
    while (1) {
        xQueueReceive(cmdQueue, &ipiCmdInfo, portMAX_DELAY);

        // USND_L("%s(), ipiCmdInfo.ipi_msgid=%d", __FUNCTION__, ipiCmdInfo.ipi_msgid);
        switch (ipiCmdInfo.ipi_msgid) {
            case USND_TASK_MSGID_ON_DONE:
            {
#ifdef ULTRASOUND_DEMO_LOOPBACK
                uSndProc.playbackDataLen = task_ultrasound_getDlSampleNumberPerFrame();
                uSndProc.playbackData = kal_pvPortMalloc(uSndProc.playbackDataLen*sizeof(int16_t));
                uSndProc.srcBufIdx = 0;
                if ( NULL == uSndProc.playbackData ) {
                    USND_E("%s(), Cannet get memory for uSndProc.playbackData in USND_TASK_MSGID_ON_DONE", __func__);
                    configASSERT(0);
                }

                USND_D("uSndProc.playbackDataLen=%d\n", uSndProc.playbackDataLen);
#endif
            }
                break;
            case USND_TASK_MSGID_OFF_DONE:
            {
#ifdef ULTRASOUND_DEMO_LOOPBACK
                vPortFree(uSndProc.playbackData);
                uSndProc.playbackDataLen = 0;
#endif
            }
                break;
            case USND_TASK_MSGID_PROCESS:
            {
#ifdef ULTRASOUND_DEMO_LOOPBACK
                uint32_t i;
                // uint32_t samples = task_ultrasound_getUlSampleNumberPerFrame();
                uint32_t samples = task_ultrasound_getUlSampleNumber();
                uint32_t firstLen, secondLen;
                int16_t *ulData = task_ultrasound_getUlData();
                if (uSndProc.srcBufIdx + uSndProc.playbackDataLen <= samples) {
                    firstLen = uSndProc.playbackDataLen;
                    secondLen = 0;
                } else {
                    firstLen = samples - uSndProc.srcBufIdx;
                    secondLen = uSndProc.playbackDataLen - firstLen;
                }
                // do somthing
                for ( i = 0; i < firstLen; i++ ) {
                    uSndProc.playbackData[i] = ulData[i + uSndProc.srcBufIdx];
                }
                uSndProc.srcBufIdx += firstLen;
                if (uSndProc.srcBufIdx >= samples) {
                    uSndProc.srcBufIdx = 0;
                }
                for ( i = 0; i < secondLen; i++ ) {
                    uSndProc.playbackData[i+firstLen] = ulData[i + uSndProc.srcBufIdx];
                }
                uSndProc.srcBufIdx += secondLen;

                task_ultrasound_writeDlData(uSndProc.playbackData, uSndProc.playbackDataLen);
#endif
            }
                break;
            case USND_TASK_MSGID_DLMIX:
            {
#if 0
                uint32_t i, j, r;
                uint32_t samples = task_ultrasound_getMddlSphSampleNumberPerFrame();
                int16_t *sphData = task_ultrasound_getMddlSphData();

                r = uSndProc.playbackDataLen/samples;
                for ( i = 0; i < (samples); i++ ) {
                    for ( j = 0; j < r; j++ ) {
                        uSndProc.playbackData[i*r+j] = sphData[i];
                    }
                }

                task_ultrasound_writeDlData(uSndProc.playbackData, uSndProc.playbackDataLen);
#endif
            }
                break;
            default:
                break;
        }
    }
}

void uSnd_Dl_Handler(void *param) {
    uSndServCmd_T msg;
    msg.ipi_msgid = USND_TASK_MSGID_DLMIX;

    if ( pdPASS != xQueueSend(cmdQueue, &msg, 0) ) {
        USND_E("%s, fail to send DL handler!", __FUNCTION__);
    }
}

void uSnd_Ul_Handler(void *param) {
    uSndServCmd_T msg;
    msg.ipi_msgid = USND_TASK_MSGID_PROCESS;

    if ( pdPASS != xQueueSend(cmdQueue, &msg, 0) ) {
        USND_E("%s, fail to send UL handler!", __FUNCTION__);
    }
}
/***********************************************************************************
** uSnd_ipi_callback - message handler from AP side
************************************************************************************/

void uSnd_Serv_callback(int id, void *data, unsigned int len) {
}

/***********************************************************************************
** uSnd_ipi_callback - message handler from AP side
************************************************************************************/
static void uSnd_serv_init(void) {
    BaseType_t xReturn = pdFAIL;
    xReturn = kal_xTaskCreate(
        uSnd_serv_task,
        "uSS",
        250,
        (void *)0,
        3,
        &xUSSHandle);
    AUD_ASSERT(xReturn == pdPASS);
    AUD_ASSERT(xUSSHandle);
#ifdef ULTRASOUND_DEMO_LOOPBACK
    task_ultrasound_setDlHandler(uSnd_Dl_Handler, NULL);
    task_ultrasound_setUlHandler(uSnd_Ul_Handler, NULL);
#endif
}

static void uSnd_serv_deinit(void) {
#if (INCLUDE_vTaskDelete == 1)
    if (xUSSHandle) {
        vTaskDelete(xUSSHandle);
    }
    xUSSHandle = NULL;
#endif
}
