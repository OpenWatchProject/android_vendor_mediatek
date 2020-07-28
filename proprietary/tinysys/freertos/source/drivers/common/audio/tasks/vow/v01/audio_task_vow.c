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

#include <FreeRTOS.h>
#include <stdarg.h>
#include <interrupt.h>
#include <semphr.h>
#include <task.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <mt_reg_base.h>
#include <dma.h>
#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#endif
#include <feature_manager.h>
// #include <wrapped_errors.h>
#include <platform_mtk.h>
#include "vow_ipi_message.h"
#include "audio_task_vow_params.h"
#include "audio_task_vow.h"
#include "vow_hw.h"
#include "vow_hw_reg.h"
#include "swVAD.h"
#include "vowAPI_SCP.h"
#include "mt_gpt.h"
#if SMART_AAD_MIC_SUPPORT
#include "eint.h"
#endif


/*==============================================================================
 *                     MACRO
 *============================================================================*/

#define LOCAL_TASK_STACK_SIZE (512)
#define LOCAL_TASK_NAME "vow"
#define LOCAL_TASK_PRIORITY (2)


/*==============================================================================
 *                     private global members
 *============================================================================*/
static struct {
    vow_status_t             vow_status;
    vow_mode_t               vow_mode;
    short                    read_idx;
    short                    write_idx;
    bool                     pre_learn;
    bool                     record_flag;
    bool                     debug_P1_flag;
    bool                     debug_P2_flag;
    bool                     dmic_low_power;
    bool                     dc_removal_enable;
    char                     dc_removal_order;
    char                     recog_working;
    char                     dvfs_mode;
    bool                     smart_device_flag;
    int                      drop_count;
    int                      drop_frame;
    int                      apreg_addr;
    int                      samplerate;
    int                      data_length;
    int                      noKeywordCount;
    int                      smart_device_eint_id;
#if (__SEAMLESS_RECORD_SUPPORT__)
    short                    read_buf_idx;
    bool                     seamless_record;
    bool                     sync_data_ready;
#endif  // #if (__SEAMLESS_RECORD_SUPPORT__)
    int                      pcmdump_cnt;
    vow_model_t              vow_model_event;
} vowserv;


short vowserv_buf_voice[VOW_LENGTH_VOICE] __attribute__((aligned(4)));
short vowserv_buf_speaker[VOW_LENGTH_SPEAKER] __attribute__((aligned(4)));

int dma_ch = VOW_DMA_CH;

int ipi_send_cnt = 0x00;

SemaphoreHandle_t xWakeVowSemphr;
SemaphoreHandle_t xVowMutex;
static void vow_task(void *pvParameters);
static void vow_getModel_speaker(void);

typedef void (*VOW_MODEL_HANDLER)(void);

#if PHASE_DEBUG
unsigned int test_cnt = 0;
#endif  // #if PHASE_DEBUG
unsigned int task_entry_cnt = 0;
unsigned int max_task_entry_cnt = 0;

#if TIMES_DEBUG
volatile unsigned int *ITM_CONTROL = (unsigned int *)0xE0000E80;
volatile unsigned int *DWT_CONTROL = (unsigned int *)0xE0001000;
volatile unsigned int *DWT_CYCCNT = (unsigned int *)0xE0001004;
volatile unsigned int *DEMCR = (unsigned int *)0xE000EDFC;
unsigned int start_recog_time = 0;
unsigned int end_recog_time = 0;
unsigned int max_recog_time = 0;
unsigned int start_task_time = 0;
unsigned int end_task_time = 0;
unsigned int max_task_time = 0;
unsigned int swvad_flag = 0;
unsigned int total_recog_time = 0;
unsigned int avg_cnt = 0;

#define CPU_RESET_CYCLECOUNTER() \
do { \
*DEMCR = *DEMCR | 0x01000000; \
*DWT_CYCCNT = 0; \
*DWT_CONTROL = *DWT_CONTROL | 1 ; \
} while (0)

// Test Method
// CPU_RESET_CYCLECOUNTER();
// start_time = *DWT_CYCCNT;
#endif  // #if TIMES_DEBUG


/*==============================================================================
 *                     derived functions - declaration
 *============================================================================*/


/*==============================================================================
 *                     private functions - declaration
 *============================================================================*/
static void vow_task(void *pvParameters);
// static void vow_resume_phase1_setting(void);
static void VOW_FIFO_IRQHandler(void);
int vow_model_init(void *SModel);
int vow_keyword_recognize(void *sample_input, int num_sample, int *ret_info);

/***********************************************************************************
** task_vow_init - Initial funciton in scp init
************************************************************************************/
void vow_init(void)
{
    vowserv.vow_status      = VOW_STATUS_IDLE;
    vowserv.read_idx        = VOW_LENGTH_VOICE + 1;
#if (__SEAMLESS_RECORD_SUPPORT__)
    vowserv.read_buf_idx    = VOW_LENGTH_VOICE + 1;
#endif  // #if (__SEAMLESS_RECORD_SUPPORT__)
    vowserv.write_idx       = 0;
    vowserv.pre_learn       = false;
    vowserv.record_flag     = false;
    vowserv.debug_P1_flag   = false;
    vowserv.debug_P2_flag   = false;
    vowserv.drop_frame      = VOW_DROP_FRAME_FOR_DC;
    vowserv.apreg_addr      = 0;
    vowserv.dmic_low_power  = false;
    vowserv.samplerate      = VOW_SAMPLERATE_16K;
    vowserv.dc_removal_enable = true;
    vowserv.dc_removal_order = 5;  // default
    vowserv.recog_working   = 0;
    vowserv.smart_device_flag = false;
    vowserv.smart_device_eint_id = 0;
    vowserv.pcmdump_cnt     = 0;
    vowserv.vow_model_event.func = vow_getModel_speaker;
    vowserv.vow_model_event.addr = 0;
    vowserv.vow_model_event.size = 0;
    vowserv.vow_model_event.flag = false;

    memset(vowserv_buf_speaker, 0, VOW_LENGTH_SPEAKER * sizeof(short));
    memset(vowserv_buf_voice, 0, VOW_LENGTH_VOICE * sizeof(short));

    vowserv.vow_model_event.data = (short*)vowserv_buf_speaker;


    // Enable IRQ;
    request_irq(MAD_FIFO_IRQn, VOW_FIFO_IRQHandler, "MAD_FIFO");

    vow_ipi_init();

    // Switch VOW BUCK Voltage
    vow_pwrapper_write(BUCK_VOW_CON0, 0x001C);

    // Create Mutex
    xVowMutex = xSemaphoreCreateMutex();
    if (xVowMutex == NULL) {
        // The semaphore can not be used.
        VOW_DBG("Vow_M fail\n\r");
    }

    // Create Semaphore
    xWakeVowSemphr = xSemaphoreCreateCounting(1, 0);
    if (xWakeVowSemphr == NULL) {
        // The semaphore was created failly.
        // The semaphore can not be used.
        VOW_DBG("Vow_S Fail\n\r");
    }

    // Create vow_task
    kal_xTaskCreate(vow_task, LOCAL_TASK_NAME, LOCAL_TASK_STACK_SIZE, (void *)0, LOCAL_TASK_PRIORITY, NULL);
}

/***********************************************************************************
** vow_enable - enable VOW
************************************************************************************/
void vow_enable(void)
{
    int ret = 0;
#if (__SEAMLESS_RECORD_SUPPORT__)
    unsigned int rdata;
#endif  // #if (__SEAMLESS_RECORD_SUPPORT__)

    VOW_LOG("+Enable_%x\n\r", vowserv.vow_status);
    if (vowserv.vow_status == VOW_STATUS_IDLE) {
        ret = vow_model_init(vowserv.vow_model_event.data);

        // Enable VOW FIFO
        vow_enable_fifo((samplerate_t)vowserv.samplerate, vowserv.smart_device_flag);

        xSemaphoreTake(xVowMutex, portMAX_DELAY);
        vowserv.read_idx = VOW_LENGTH_VOICE + 1;
#if (__SEAMLESS_RECORD_SUPPORT__)
        vowserv.read_buf_idx = VOW_LENGTH_VOICE + 1;
#endif  // #if (__SEAMLESS_RECORD_SUPPORT__)
        vowserv.write_idx = 0;
        vowserv.data_length = 0;
        vowserv.recog_working = 0;
        xSemaphoreGive(xVowMutex);

        vow_dc_removal(vowserv.dc_removal_enable, vowserv.dc_removal_order);

#if (__SEAMLESS_RECORD_SUPPORT__)
        rdata = vow_pwrapper_read(AFE_VOW_TOP);
        if ((rdata & 0x0800) == 0x0000) {
            vow_pmic_idleTophase1(vowserv.dmic_low_power, vowserv.dvfs_mode, vowserv.smart_device_flag);
        } else {
            vow_pmic_phase2Tophase1(vowserv.dvfs_mode, vowserv.smart_device_flag);
        }
        vowserv.sync_data_ready = true;
#else  // #if (__SEAMLESS_RECORD_SUPPORT__)
        vow_pmic_idleTophase1(vowserv.dmic_low_power, vowserv.dvfs_mode, vowserv.smart_device_flag);
#endif  // #if (__SEAMLESS_RECORD_SUPPORT__)
        VOW_LOG("debug P1:%d, P2:%d\n", vowserv.debug_P1_flag, vowserv.debug_P2_flag);
        if (vowserv.debug_P1_flag) {
            vow_force_phase1();
        }
        if (((vowserv.pre_learn) || (vowserv.debug_P2_flag)) && (!vowserv.debug_P1_flag)) {
            // AUD_LOG_D("Enable, prelearn\n");
            vow_force_phase2(vowserv.dvfs_mode, vowserv.smart_device_flag);
        }

        vowserv.drop_count = 0;
        vowserv.vow_status = VOW_STATUS_SCP_REG;
        vowserv.pcmdump_cnt = 0;
#if SMART_AAD_MIC_SUPPORT
       if (vowserv.smart_device_flag == true) {
           mt_eint_dump_all_config();
           mt_eint_ack(vowserv.smart_device_eint_id);
           mt_eint_unmask(vowserv.smart_device_eint_id);
       }
#endif

        // For SW VAD init
        initSwVad();
        vowserv.noKeywordCount = 0;
#if PHASE_DEBUG
        test_cnt = 0;
#endif  // #if PHASE_DEBUG

        task_entry_cnt = 0;
#if TIMES_DEBUG
        total_recog_time = 0;
        avg_cnt = 0;
        start_recog_time = 0;
        end_recog_time = 0;
        max_recog_time = 0;
        start_task_time = 0;
        end_task_time = 0;
        max_task_time = 0;
        CPU_RESET_CYCLECOUNTER();
#endif  // #if TIMES_DEBUG
        // set wakeup source
        scp_wakeup_src_setup(MAD_FIFO_WAKE_CLK_CTRL, 1);
        unmask_irq(MAD_FIFO_IRQn);

        VOW_DBG("-Enable_%x_%x\n\r", vowserv.vow_status, ret);
    } else {
#if (__SEAMLESS_RECORD_SUPPORT__)
        rdata = vow_pwrapper_read(AFE_VOW_TOP);
        VOW_LOG("vow_status not in idle, AFE_VOW_TOP:%x\n\r", rdata);
        if ((rdata & 0x0800) == 0x0000) {
            VOW_LOG("ENABLE: IDLE to P1\n\r");
            vow_pmic_idleTophase1(vowserv.dmic_low_power, vowserv.dvfs_mode, vowserv.smart_device_flag);
        } else {
            if (vowserv.seamless_record == false) {
                VOW_LOG("ENABLE: P2 to Idle To Phase1\n\r");
                vow_pmic_idle(vowserv.smart_device_flag);
                vow_pmic_idleTophase1(vowserv.dmic_low_power, vowserv.dvfs_mode, vowserv.smart_device_flag);
            }
        }
#endif  // #if (__SEAMLESS_RECORD_SUPPORT__)
    }
}

/***********************************************************************************
s** vow_disable - disable VOW
************************************************************************************/
void vow_disable(void)
{
    VOW_LOG("+Disable_%x\n\r", vowserv.vow_status);
    if (vowserv.vow_status != VOW_STATUS_IDLE) {
        // Clear wakeup source
        scp_wakeup_src_setup(MAD_FIFO_WAKE_CLK_CTRL, 0);
        mask_irq(MAD_FIFO_IRQn);
#if SMART_AAD_MIC_SUPPORT
       if (vowserv.smart_device_flag == true) {
           // Clear wakeup source
           scp_wakeup_src_setup(EINT_WAKE_CLK_CTRL, 0);
           mt_eint_mask(vowserv.smart_device_eint_id);
       }
#endif

        vow_disable_fifo();

        vow_pmic_idle(vowserv.smart_device_flag);
        vow_dc_removal(false, 0);
        vowserv.vow_status = VOW_STATUS_IDLE;
        xSemaphoreTake(xVowMutex, portMAX_DELAY);
        vowserv.read_idx = VOW_LENGTH_VOICE + 1;
#if (__SEAMLESS_RECORD_SUPPORT__)
        vowserv.read_buf_idx = VOW_LENGTH_VOICE + 1;
#endif  // #if (__SEAMLESS_RECORD_SUPPORT__)
        vowserv.write_idx = 0;
        vowserv.data_length = 0;
        xSemaphoreGive(xVowMutex);

        // For SW VAD
        vowserv.noKeywordCount = 0;

#if PHASE_DEBUG
        test_cnt = 0;
#endif  // #if PHASE_DEBUG

        VOW_DBG("-Disable_%x\n\r", vowserv.vow_status);
    }
}

/***********************************************************************************
** vow_sync_data_ready - for sync every pcm dump package
************************************************************************************/
void vow_sync_data_ready(void)
{
#if (__SEAMLESS_RECORD_SUPPORT__)
    vowserv.sync_data_ready = true;
#endif
}

/***********************************************************************************
** vow_getModel_speaker - Get speaker model informations through DMA
************************************************************************************/
static void vow_getModel_speaker(void)
{
    char *ptr8;
    unsigned int ret;

    VOW_DBG("GetModel S_%x %x %x\n", vowserv.vow_model_event.addr,
                                       vowserv.vow_model_event.size,
                                       (int)&vowserv_buf_speaker[0]);
    ret = vow_dma_transaction((uint32_t)&vowserv_buf_speaker[0],
                              (uint32_t)vowserv.vow_model_event.addr,
                              (uint32_t)vowserv.vow_model_event.size);
    if (ret !=  DMA_RESULT_DONE) {
         VOW_ERR("%s(), DMA fail:%x\n", __func__, ret);
    }
    // To do: DMA Speaker Model into vowserv_buf_speaker
    ptr8 = (char*)vowserv_buf_speaker;

    VOW_DBG("check_%x %x %x %x %x %x\n", *(char*)&ptr8[0], *(char*)&ptr8[1],
                                           *(char*)&ptr8[2], *(char*)&ptr8[3],
                                           *(short*)&ptr8[160], *(int*)&ptr8[7960]);
}

/***********************************************************************************
** vow_setapreg_addr - Get noise model informations through DMA
************************************************************************************/
void vow_setapreg_addr(unsigned int addr)
{
    VOW_DBG("SetAPREG_%x_%x\n", addr, vowserv.apreg_addr);
    if (vowserv.apreg_addr == 0) {
        vowserv.apreg_addr = ap_to_scp(addr);  // AP's address
        VOW_DBG("SetAPREG_done\n");
    }
}

/***********************************************************************************
** vow_gettcmreg_addr - Get noise model informations through DMA
************************************************************************************/
unsigned int vow_gettcmreg_addr(void)
{
    unsigned int tcm_addr;
    tcm_addr = (unsigned int)&vowserv_buf_voice[0];  // SCP's address
    VOW_DBG("SetTcmAddr_%x\n", tcm_addr);
    return tcm_addr;
}

/***********************************************************************************
** vow_set_flag - Enable/Disable specific operations
************************************************************************************/
void vow_set_flag(vow_flag_t flag, short enable)
{
    switch (flag) {
        case VOW_FLAG_DEBUG:
            VOW_DBG("FgDebug_%x\n\r", enable);
            vowserv.record_flag = enable;
#if (__SEAMLESS_RECORD_SUPPORT__)
            vowserv.seamless_record = enable;
#endif  // #if (__SEAMLESS_RECORD_SUPPORT__)
            break;
        case VOW_FLAG_PRE_LEARN:
            VOW_DBG("FgPreL_%x\n\r", enable);
            vowserv.pre_learn = enable;
            break;
        case VOW_FLAG_DMIC_LOWPOWER:
            VOW_DBG("FgDmic_%x\n\r", enable);
            vowserv.dmic_low_power = enable;
            break;
        case VOW_FLAG_FORCE_PHASE1_DEBUG:
            VOW_DBG("Force Phase1 Debug:%x\n", enable);
            vowserv.debug_P1_flag = enable;
            if ((vowserv.debug_P1_flag == true) && (vowserv.debug_P2_flag == true)) {
                VOW_DBG("clear Phase2 Debug\n");
                vowserv.debug_P2_flag = false;
            }
            break;
        case VOW_FLAG_FORCE_PHASE2_DEBUG:
            VOW_DBG("Force Phase2 Debug:%x\n", enable);
            vowserv.debug_P2_flag = enable;
            if ((vowserv.debug_P1_flag == true) && (vowserv.debug_P2_flag == true)) {
                VOW_DBG("clear Phase1 Debug\n");
                vowserv.debug_P1_flag = false;
            }
            break;
        default:
            break;
    }
}

/***********************************************************************************
** vow_getEintNumber - Indicate using smart external device
************************************************************************************/
#if SMART_AAD_MIC_SUPPORT
unsigned int vow_getEintNumber(unsigned int eint_num)
{
    return (EINT_NUMBER_0 + eint_num);
}
#endif

/***********************************************************************************
** vow_setSmartDevice - Indicate using smart external device
************************************************************************************/

void vow_setSmartDevice(bool enable, unsigned int eint_num)
{
    if (enable == true) {
#if SMART_AAD_MIC_SUPPORT
        VOW_DBG("smart device enable, flag:%x\n\r", vowserv.smart_device_flag);
        if (vowserv.smart_device_flag == false) {
            vowserv.smart_device_flag = true;
            vowserv.smart_device_eint_id = vow_getEintNumber(eint_num);
            if (vowserv.smart_device_eint_id > EINT_NUMBER_17) {
                VOW_ERR("eint id is out of order!\n\r", vowserv.smart_device_eint_id);
            }
            mt_eint_registration(vowserv.smart_device_eint_id,
                                 EDGE_SENSITIVE,
                                 HIGH_LEVEL_TRIGGER,
                                 vow_smartdevice_eint_isr,
                                 EINT_INT_MASK,
                                 EINT_INT_AUTO_UNMASK_OFF);
            scp_wakeup_src_setup(EINT_WAKE_CLK_CTRL, 1);
            vowserv.samplerate = VOW_SAMPLERATE_16K;
            mt_eint_dis_hw_debounce(vowserv.smart_device_eint_id);
            VOW_DBG("[VOW]mt_eint_read_status %x\n\r", vowserv.smart_device_eint_id);
        }
#endif
    } else if (enable == false) {
        VOW_DBG("smart device disable\n\r");
        vowserv.smart_device_flag = false;
        vowserv.samplerate = VOW_SAMPLERATE_16K;
    }
    VOW_DBG("Set smart_device_flag %x, id=%d\n\r", vowserv.smart_device_flag, vowserv.smart_device_eint_id);
}


/***********************************************************************************
** vow_setmode - handle set mode request
************************************************************************************/
void vow_setmode(vow_mode_t mode)
{
    VOW_LOG("SetMode,%x %x\n\r", vowserv.vow_mode, mode);
    vowserv.vow_mode = mode;
}

/***********************************************************************************
** vow_setModel - set model information
************************************************************************************/
void vow_setModel(vow_event_info_t type, int id, int addr, int size)
{
    VOW_DBG("SetModel+ %x %x %x %x\n\r", type, id, addr, size);

    // AUD_ASSERT(type == VOW_MODEL_SPEAKER);
    // AUD_ASSERT((size >> 1) <= VOW_LENGTH_SPEAKER);
    if (type != VOW_MODEL_SPEAKER) {
        return;
    }

    VOW_DBG("cur model addr:0x%x\n", vowserv.vow_model_event.addr);

    vowserv.vow_model_event.addr = ap_to_scp(addr);
    vowserv.vow_model_event.size = size;
    vowserv.vow_model_event.flag = true;

    VOW_DBG("SetModel- %x %x %x %x\n\r", type, id, addr, size);

    xSemaphoreGive(xWakeVowSemphr);
}

/***********************************************************************************
** vow_model_init - Initialize SWIP with different model_type
************************************************************************************/
int vow_model_init(void *SModel)
{
    int ret = -1;

#if (__VOW_GOOGLE_SUPPORT__)
    VOW_DBG("Google_M init\n\r");
    ret = GoogleHotwordDspInit((void *)SModel);
#elif(__VOW_MTK_SUPPORT__)
    VOW_DBG("MTK_M init\n\r");
    ret = TestingInit_Model((char *)SModel);
    TestingInit_Table(0);
#else  // #if (__VOW_GOOGLE_SUPPORT__)
    VOW_ERR("Not Support\n\r");
#endif  // #if (__VOW_GOOGLE_SUPPORT__)
    return ret;
}

/***********************************************************************************
** vow_keyword_Recognize - use SWIP to recognize keyword with different model_type
************************************************************************************/
int vow_keyword_recognize(void *sample_input, int num_sample, int *ret_info)
{
    int ret = -1;

#if (__VOW_GOOGLE_SUPPORT__)
    ret = GoogleHotwordDspProcess(sample_input, num_sample, ret_info);
    if (ret == 1) {
        // Reset in order to resume detection.
        GoogleHotwordDspReset();
    } else {  // ret = 0
        // call MTK SW VAD, check if in "no-sound" place
        vowserv.noKeywordCount++;
#if TIMES_DEBUG
        swvad_flag = 1;
#endif  // #if TIMES_DEBUG
        if (returnToPhase1BySwVad((short *)sample_input, vowserv.noKeywordCount)) {
            ret = 2;  // Become no sound, back to phase1
        }
#if TIMES_DEBUG
        swvad_flag = 0;
#endif  // #if TIMES_DEBUG
    }
#elif(__VOW_MTK_SUPPORT__)
    (void)num_sample;
    ret = onTesting((short *)sample_input, ret_info);  // 0: not thing, 1: pass, 2: fail(no-sound)
    if (ret == 0) {
        vowserv.noKeywordCount++;
#if TIMES_DEBUG
        swvad_flag = 1;
#endif  // #if TIMES_DEBUG
        if (returnToPhase1BySwVad((short *)sample_input, vowserv.noKeywordCount)) {
            ret = 2;  // Become no sound, back to phase1
        }
#if TIMES_DEBUG
        swvad_flag = 0;
#endif  // #if TIMES_DEBUG
    }
    if ((ret == 1) || (ret == 2)) {
        resetSwVadPara();
        vowserv.noKeywordCount = 0;
    }
#else  // #if (__VOW_GOOGLE_SUPPORT__)
    VOW_ERR("Not Support\n\r");
#endif  // #if (__VOW_GOOGLE_SUPPORT__)
    return ret;
}

/***********************************************************************************
** vow_resume_phase1 - handle new control flow when P2 to P1
************************************************************************************/
#if 0
static void vow_resume_phase1_setting(void)
{
    mask_irq(MAD_FIFO_IRQn);
    vowserv.read_idx = VOW_LENGTH_VOICE + 1;
#if (__SEAMLESS_RECORD_SUPPORT__)
    vowserv.read_buf_idx = VOW_LENGTH_VOICE + 1;
#endif  // #if (__SEAMLESS_RECORD_SUPPORT__)
    vowserv.write_idx = 0;
    vowserv.data_length = 0;
    unmask_irq(MAD_FIFO_IRQn);
}
#endif

/***********************************************************************************
** task_vow_working - main task content
************************************************************************************/
static void vow_task(void *pvParameters)
{
    int ret, RetInfo;  //, PreambleMs;
    int w_idx, r_idx, data_amount;
    int LoopAgain = 0;
    vow_pcmdump_info_t pcmdump_info;
#if (__SEAMLESS_RECORD_SUPPORT__)
    unsigned int Offset_Size;
    unsigned int Tx_Size;
    int delay_count = 0;
    unsigned int Rdy2Send_Offset = 0;
    unsigned int Rdy2Send_Size = 0;
#else  // #if (__SEAMLESS_RECORD_SUPPORT__)
    bool pcmdump_start_remind_kernel = false;
    unsigned int pingpong_offset;
#endif  // #if (__SEAMLESS_RECORD_SUPPORT__)
#if TIMES_DEBUG
    unsigned int Temp_recog_time = 0;
    unsigned int Temp_task_time = 0;
#endif  // #if TIMES_DEBUG

    while (1) {
        if (LoopAgain == 1) {
            // VOW_DBG("LoopAg\n\r");

            task_entry_cnt++;
            if (task_entry_cnt > 5) {
                vTaskDelay(4);
            }
#if TIMES_DEBUG
            if (task_entry_cnt > max_task_entry_cnt) {
                max_task_entry_cnt = task_entry_cnt;
            }
#endif
            LoopAgain = 0;
        } else {
            xSemaphoreTake(xWakeVowSemphr, portMAX_DELAY);
            task_entry_cnt = 0;
        }
        // VOW_LOG("vow_task\n\r");



        if (vowserv.vow_model_event.flag) {
            vowserv.vow_model_event.func();
            vowserv.vow_model_event.flag = false;
            vow_ipi_sendmsg(SCP_IPIMSG_SET_VOW_MODEL_ACK, 0, VOW_IPI_SUCCESS, 0, 1);
        }

        if ((vowserv.vow_status != VOW_STATUS_IDLE) && (vowserv.data_length >= VOW_SAMPLE_NUM_FRAME)) {
#if PHASE_DEBUG
            test_cnt++;
            if (test_cnt == 1)
                VOW_LOG("==P2==\n\r");
#endif
#if TIMES_DEBUG
            start_task_time = *DWT_CYCCNT;
#endif  // #if TIMES_DEBUG
            xSemaphoreTake(xVowMutex, portMAX_DELAY);
            if (vowserv.read_idx >= VOW_LENGTH_VOICE) {
                vowserv.read_idx = 0;
            }
#if (__SEAMLESS_RECORD_SUPPORT__)
            if (vowserv.read_buf_idx >= VOW_LENGTH_VOICE) {
                vowserv.read_buf_idx = 0;
            }
#endif  // #if (__SEAMLESS_RECORD_SUPPORT__)
            // Get current read_index
            r_idx = vowserv.read_idx;
#if (__SEAMLESS_RECORD_SUPPORT__)
            w_idx = vowserv.write_idx;
#endif
            // Update read_index
            if (vowserv.read_idx < VOW_LENGTH_VOICE) {
                vowserv.read_idx += VOW_SAMPLE_NUM_FRAME;
            }
            if (vowserv.data_length >= VOW_SAMPLE_NUM_FRAME) {
                vowserv.data_length -= VOW_SAMPLE_NUM_FRAME;
            }
            xSemaphoreGive(xVowMutex);

            if (vowserv.record_flag) {
#if (__SEAMLESS_RECORD_SUPPORT__)
                if (vowserv.seamless_record && delay_count >= SEAMLESS_DELAY) {
                    vowserv.seamless_record = false;
                    if (vowserv.sync_data_ready) {  // seamless record don't need this flag help
                        vowserv.sync_data_ready = false;
                    }
                    delay_count = 0;
                    // upload seamless data into Kernel
                    Rdy2Send_Offset = 0;
                    if (w_idx >= r_idx) {
                        dvfs_enable_DRAM_resource(VOW_MEM_ID);
                        Offset_Size = ((VOW_LENGTH_VOICE - w_idx) << 1);
                        // DMA from W_idx to bottom
                        ret = vow_dma_transaction((uint32_t)vowserv.apreg_addr,
                                                  (uint32_t)&vowserv_buf_voice[w_idx],
                                                  Offset_Size);
                        if (ret !=  DMA_RESULT_DONE) {
                            VOW_ERR("%s(), DMA bottom fail:%x\n", __func__, ret);
                        }
                        // DMA from top to r_idx
                        ret = vow_dma_transaction((uint32_t)vowserv.apreg_addr + Offset_Size,
                                                  (uint32_t)&vowserv_buf_voice[0],
                                                  ((r_idx + VOW_SAMPLE_NUM_FRAME) << 1));
                        if (ret !=  DMA_RESULT_DONE) {
                            VOW_ERR("%s(), DMA top fail:%x\n", __func__, ret);
                        }

                        Rdy2Send_Size = Offset_Size + ((r_idx + VOW_SAMPLE_NUM_FRAME) << 1);
                    } else {
                        Offset_Size = ((r_idx - w_idx + VOW_SAMPLE_NUM_FRAME) << 1);
                        ret = vow_dma_transaction((uint32_t)vowserv.apreg_addr,
                                                  (uint32_t)&vowserv_buf_voice[w_idx],
                                                  Offset_Size);
                        if (ret !=  DMA_RESULT_DONE) {
                            VOW_ERR("%s(), DMA fail:%x\n", __func__, ret);
                        }
                        Rdy2Send_Size = Offset_Size;
                    }
                    vowserv.read_buf_idx = r_idx + VOW_SAMPLE_NUM_FRAME;
                    VOW_LOG("W:0x%x, R:0x%x, Total:0x%xByte\n\r", w_idx, r_idx, Rdy2Send_Size);
                } else if (vowserv.seamless_record == true) {
                    delay_count++;
                    Rdy2Send_Size = 0;
                    Rdy2Send_Offset = 0;
                } else {
                    // upload pcm dump data into Kernel
                    if (vowserv.sync_data_ready) {
                        vowserv.sync_data_ready = false;
                        if (r_idx >= vowserv.read_buf_idx) {
                            Tx_Size = r_idx - vowserv.read_buf_idx + VOW_LENGTH_VOWREG;
                            ret = vow_dma_transaction((uint32_t)vowserv.apreg_addr,
                                                      (uint32_t)&vowserv_buf_voice[vowserv.read_buf_idx],
                                                      (Tx_Size << 1));
                            if (ret !=  DMA_RESULT_DONE) {
                                VOW_ERR("%s(), DMA fail:%x\n", __func__, ret);
                            }
                        } else {
                            Tx_Size = (VOW_LENGTH_VOICE - vowserv.read_buf_idx) + r_idx + VOW_LENGTH_VOWREG;
                            Offset_Size = ((VOW_LENGTH_VOICE - vowserv.read_buf_idx) << 1);
                            ret = vow_dma_transaction((uint32_t)vowserv.apreg_addr,
                                                      (uint32_t)&vowserv_buf_voice[vowserv.read_buf_idx],
                                                      Offset_Size);
                            if (ret !=  DMA_RESULT_DONE) {
                                VOW_ERR("%s(), DMA bottom fail:%x\n", __func__, ret);
                            }
                            ret = vow_dma_transaction((uint32_t)vowserv.apreg_addr + Offset_Size,
                                                      (uint32_t)&vowserv_buf_voice[0],
                                                      ((r_idx + VOW_LENGTH_VOWREG) << 1));
                            if (ret !=  DMA_RESULT_DONE) {
                                VOW_ERR("%s(), DMA bottom fail:%x\n", __func__, ret);
                            }
                        }
                        Rdy2Send_Size = (Tx_Size << 1);
                        Rdy2Send_Offset = 0;
                        vowserv.read_buf_idx += Tx_Size;
                        VOW_LOG("R:0x%x, R_buf:0x%x, Total:0x%xByte\n\r", r_idx, vowserv.read_buf_idx, Total_Size);
                    }
                }
                if (Rdy2Send_Size > 0) {
                    ipi_send_cnt = 0;
                    pcmdump_info.dump_buf_offset = Rdy2Send_Offset;  // DMA offset
                    pcmdump_info.dump_size = Rdy2Send_Size;  // DMA size (Byte)
                    while (vow_ipi_sendmsg(SCP_IPIMSG_VOW_DATAREADY, &pcmdump_info, VOW_IPI_SUCCESS,
                           sizeof(vow_pcmdump_info_t), 1) == false) {
                        udelay(1000);  // delay 1ms
                        ipi_send_cnt++;
                        if (ipi_send_cnt > VOW_IPI_TIMEOUT) {
                            VOW_LOG("IPI SEND FAIL, BYPASS:0x%x, %d\n\r", SCP_IPIMSG_VOW_DATAREADY, ipi_send_cnt);
                            break;
                        }
                    }
                    if (ipi_send_cnt > 0) {
                        VOW_LOG("Timeout_cnt:%d\n\r", ipi_send_cnt);
                    }
                }
#else  // #if (__SEAMLESS_RECORD_SUPPORT__)
                if (r_idx < VOW_LENGTH_VOICE) {
                    ret = vow_dma_transaction((uint32_t)vowserv.apreg_addr +
                                              (vowserv.pcmdump_cnt * (VOW_LENGTH_VOWREG << 1)),
                                              (uint32_t)&vowserv_buf_voice[r_idx],
                                              (VOW_LENGTH_VOWREG << 1));
                    if (ret !=  DMA_RESULT_DONE) {
                        VOW_ERR("%s(), DMA fail:%x\n", __func__, ret);
                    }
                } else {
                    VOW_ERR("1.error read index: %d\n\r", r_idx);
                }
                vowserv.pcmdump_cnt++;
                pcmdump_start_remind_kernel = false;
                // if pcmdump_cnt is half or full, then send IPI to kernel
                if ((vowserv.pcmdump_cnt == (PCMDUMP_BUF_CNT >> 1))
                 || (vowserv.pcmdump_cnt == PCMDUMP_BUF_CNT)) {
                    pingpong_offset = (vowserv.pcmdump_cnt - (PCMDUMP_BUF_CNT >> 1)) * (VOW_LENGTH_VOWREG << 1);
                    pcmdump_start_remind_kernel = true;
                }

                if (vowserv.pcmdump_cnt == PCMDUMP_BUF_CNT) {
                    vowserv.pcmdump_cnt = 0;
                }

                if (pcmdump_start_remind_kernel == true) {
                    ipi_send_cnt = 0;
                    pcmdump_info.dump_buf_offset = pingpong_offset;  // DMA offset
                    pcmdump_info.dump_size = (PCMDUMP_BUF_CNT >> 1) * (VOW_LENGTH_VOWREG << 1);  // DMA size (Byte)
                    VOW_LOG("offset: 0x%x, leng: 0x%x\n\r", pcmdump_info.dump_buf_offset, pcmdump_info.dump_size);
                    while (vow_ipi_sendmsg(SCP_IPIMSG_VOW_DATAREADY, &pcmdump_info, VOW_IPI_SUCCESS,
                           sizeof(vow_pcmdump_info_t), 1) == false) {
                        udelay(1000);  // delay 1ms
                        ipi_send_cnt++;
                        if (ipi_send_cnt > VOW_IPI_TIMEOUT) {
                            VOW_DBG("IPI SEND FAIL, BYPASS:0x%x, %d\n\r", SCP_IPIMSG_VOW_DATAREADY, ipi_send_cnt);
                            break;
                        }
                    }
                    if (ipi_send_cnt > 0) {
                        VOW_LOG("Timeout_cnt:%d\n\r", ipi_send_cnt);
                    }
                }
#endif  // #if (__SEAMLESS_RECORD_SUPPORT__)
            }

#if VOW_HAS_SWIP

#if TIMES_DEBUG
            start_recog_time = *DWT_CYCCNT;
#endif  // #if TIMES_DEBUG
            ret = VOW_RESULT_NOTHING;
            if (r_idx < VOW_LENGTH_VOICE) {
                ret = vow_keyword_recognize((void *)&vowserv_buf_voice[r_idx], VOW_LENGTH_VOWREG, &RetInfo);
            } else {
                VOW_ERR("2.error read index: %d\n\r", r_idx);
            }
#if TIMES_DEBUG
            end_recog_time = *DWT_CYCCNT;
#endif  // #if TIMES_DEBUG
            if (ret == VOW_RESULT_PASS) {
                VOW_LOG("Recog Ok: %x\n\r", RetInfo);
                if ((vowserv.record_flag == false) && (vowserv.debug_P2_flag == false)) {
                    // VOW_LOG("WakeToP1\n\r");
#if PHASE_DEBUG
                    VOW_LOG("P2_cnt:%d\n\r", test_cnt);
                    test_cnt = 0;
#endif  // #if PHASE_DEBUG
#if !(__SEAMLESS_RECORD_SUPPORT__)
                    vow_pmic_phase2Tophase1(vowserv.dvfs_mode, vowserv.smart_device_flag);
                    // vow_resume_phase1_setting();
#endif  // #if !(__SEAMLESS_RECORD_SUPPORT__)
                }
                ipi_send_cnt = 0;
                while (vow_ipi_sendmsg(SCP_IPIMSG_VOW_RECOGNIZE_OK, &RetInfo, VOW_IPI_SUCCESS, 4, 1) == false) {
                    udelay(1000);  // delay 1ms
                    ipi_send_cnt++;
                    if (ipi_send_cnt > VOW_IPI_TIMEOUT) {
                        VOW_DBG("IPI SEND FAIL, BYPASS:0x%x, %d\n\r", SCP_IPIMSG_VOW_RECOGNIZE_OK, ipi_send_cnt);
                        break;
                    }
                }
                if (ipi_send_cnt > 0) {
                    VOW_LOG("Timeout_cnt:%d\n\r", ipi_send_cnt);
                }
            } else if (ret == VOW_RESULT_FAIL) {
                if ((vowserv.record_flag == false) && (vowserv.debug_P2_flag == false)) {
#if PHASE_DEBUG
                    VOW_LOG("P2_cnt:%d\n\r", test_cnt);
                    test_cnt = 0;
#endif  // #if PHASE_DEBUG
                    vow_pmic_phase2Tophase1(vowserv.dvfs_mode, vowserv.smart_device_flag);
                    // vow_resume_phase1_setting();
                }

            } else {
                // VOW_DBG("not thing: %x\n\r", RetInfo);
            }
#else  // #if VOW_HAS_SWIP
            (void)ret;
            VOW_LOG("Bypass\n");
            RetInfo = 0;  // reg ok
            (void)RetInfo;
            vow_ipi_sendmsg(SCP_IPIMSG_VOW_RECOGNIZE_OK, &RetInfo, VOW_IPI_SUCCESS, 4, 1);
#endif  // #if VOW_HAS_SWIP
            xSemaphoreTake(xVowMutex, portMAX_DELAY);
            w_idx = vowserv.write_idx;  // updated
            r_idx = vowserv.read_idx;
            xSemaphoreGive(xVowMutex);
            // VOW_LOG("R: %d/ W: %d\n\r", r_idx, w_idx);
            if (r_idx > w_idx) {
                data_amount = (VOW_LENGTH_VOICE - r_idx + w_idx);
            } else {
                data_amount = w_idx - r_idx;
            }
            if (data_amount >= VOW_SAMPLE_NUM_FRAME) {
                LoopAgain = 1;
            }
#if SMART_AAD_MIC_SUPPORT
            if ((ret == VOW_RESULT_PASS || ret == VOW_RESULT_FAIL) && (vowserv.smart_device_flag == true)) {
                VOW_LOG("WAIT EINT\n\r");
                mt_eint_ack(vowserv.smart_device_eint_id);
                mt_eint_unmask(vowserv.smart_device_eint_id);
            }
#endif

#if TIMES_DEBUG
            end_task_time = *DWT_CYCCNT;
            if (end_recog_time > start_recog_time) {
                Temp_recog_time = (end_recog_time - start_recog_time);
            } else {
                Temp_recog_time = max_recog_time;  // for DWT_CYCCNT overflow, giveup this time
            }
            if (Temp_recog_time > max_recog_time) {
                max_recog_time = Temp_recog_time;
            }
            if (Temp_recog_time > 5600000) {  // about 50msec = 50000 * 112
                VOW_ERR("ERROR!!! Recognition too long: %d\n\r", Temp_recog_time);
            }
            if (end_task_time > start_task_time) {
                Temp_task_time = (end_task_time - start_task_time);
            } else {
                Temp_task_time = max_task_time;  // for DWT_CYCCNT overflow, giveup this time
            }
            VOW_LOG("[0x%x], <0x%x>\n", Temp_recog_time, Temp_task_time);
            if (avg_cnt == 1100) {
                avg_cnt++;
                total_recog_time /= 1000;
            } else if (avg_cnt > 1100) {
                VOW_LOG("AVG:0x%x\n", total_recog_time);
            } else if (avg_cnt >= 100) {
                avg_cnt++;
                total_recog_time += Temp_recog_time;
                VOW_LOG("avg_cnt:0x%x\n", avg_cnt);
            } else {
                avg_cnt++;
            }
            if (Temp_task_time > max_task_time) {
                max_task_time = Temp_task_time;
            }
#endif
        }
    }
}


/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        ISR Handler
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
/***********************************************************************************
** vow_smartdevice_eint_isr - Smart device EINT handler
************************************************************************************/
#if SMART_AAD_MIC_SUPPORT
void vow_smartdevice_eint_isr(int index)
{
    vow_smartdevice_get_eint(vowserv.smart_device_flag);
}
#endif

/***********************************************************************************
** VOW_FIFO_IRQHandler - Mic Data Buffer Handler
************************************************************************************/
static void VOW_FIFO_IRQHandler(void)
{
    int data_amount;
    int w_idx;
    int r_idx;
    static BaseType_t xHigherPriorityTaskWoken;

    xHigherPriorityTaskWoken = pdFALSE;
    /*----------------------------------------------------------------------
    //  Check Status
    ----------------------------------------------------------------------*/
    if (vowserv.vow_status == VOW_STATUS_IDLE) {
        // VOW_LOG("mic_idle\n\r");
        vow_read_fifo_data(NULL, VOW_SAMPLE_NUM_IRQ);
        vow_reset_fifo_irq();
        // return INT_RESCHEDULE;
        return;
    }
    if (vowserv.drop_count < vowserv.drop_frame) {
        // VOW_LOG("drop\n\r");
        vow_read_fifo_data(NULL, VOW_SAMPLE_NUM_IRQ);
        vowserv.drop_count++;
        vow_reset_fifo_irq();
        // return INT_RESCHEDULE;
        return;
    }

    xSemaphoreTakeFromISR(xVowMutex, &xHigherPriorityTaskWoken);
    w_idx = vowserv.write_idx;
    r_idx = vowserv.read_idx;
    xSemaphoreGiveFromISR(xVowMutex, &xHigherPriorityTaskWoken);

    if ((w_idx == r_idx) && (vowserv.data_length == VOW_LENGTH_VOICE)) {
        vow_read_fifo_data(NULL, VOW_SAMPLE_NUM_IRQ);
        xSemaphoreGiveFromISR(xWakeVowSemphr, &xHigherPriorityTaskWoken);
        vow_reset_fifo_irq();
        // return INT_RESCHEDULE;
        return;
    }

    /*----------------------------------------------------------------------
    //  Read From VOW FIFO and do memcpy
    ----------------------------------------------------------------------*/
    xSemaphoreTakeFromISR(xVowMutex, &xHigherPriorityTaskWoken);
    w_idx = vowserv.write_idx;
    xSemaphoreGiveFromISR(xVowMutex, &xHigherPriorityTaskWoken);
    vow_read_fifo_data(&vowserv_buf_voice[vowserv.write_idx], VOW_SAMPLE_NUM_IRQ);

    xSemaphoreTakeFromISR(xVowMutex, &xHigherPriorityTaskWoken);
    vowserv.write_idx += VOW_SAMPLE_NUM_IRQ;
    if (vowserv.write_idx >= VOW_LENGTH_VOICE) {
        vowserv.write_idx = 0;
    }
    vowserv.data_length += VOW_SAMPLE_NUM_IRQ;
    xSemaphoreGiveFromISR(xVowMutex, &xHigherPriorityTaskWoken);

    /*----------------------------------------------------------------------
    //  Calculate data amount and trigger VOW task if necessary
    ----------------------------------------------------------------------*/
    xSemaphoreTakeFromISR(xVowMutex, &xHigherPriorityTaskWoken);
    w_idx = vowserv.write_idx;  // updated
    r_idx = vowserv.read_idx;
    xSemaphoreGiveFromISR(xVowMutex, &xHigherPriorityTaskWoken);
    if (r_idx > w_idx) {
        data_amount = (VOW_LENGTH_VOICE - r_idx + w_idx);
    } else {
        data_amount = w_idx - r_idx;
    }

    if (data_amount >= VOW_SAMPLE_NUM_FRAME) {  // VOW_SAMPLE_NUM_FRAME = 160 (10msec sample)
        xSemaphoreGiveFromISR(xWakeVowSemphr, &xHigherPriorityTaskWoken);
    }

    vow_reset_fifo_irq();

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    return;
}
