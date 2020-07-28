/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#pragma once

#include "camera_custom_nvram.h"
#include "awb_mgr_if.h"
#include "ae_param.h"

#define FLASH_AAO_BUF_SIZE 54000

/* flash custom adapter */
class FlashPlatformAdapter
{
    public:
        FlashPlatformAdapter(int sensor);
        ~FlashPlatformAdapter();

        /** init/uninit */
        int init();
        int uninit();

        static FlashPlatformAdapter *getInstance(int sensor);

        static int getIspGainBase();
        static int getAfeGainBase();

        /** awb */
        int getAwbLinearOutputEn();
        int getAwbModuleVariationGain(AWB_GAIN_T *awbPreGain1Prev,
                AWB_GAIN_T *awbPreGain1Curr);
        int restoreAwbModuleVariationGain(AWB_GAIN_T *awbPreGain1Prev);
        int clearAwbModuleVariationGain(AWB_GAIN_T *awbPreGain1Curr);

        /** ae */
        int updateAePreviewParams(AE_MODE_CFG_T *aeParam, int bestInd);
        int setAeCCUOnOff(int enable);
        int setAeExpSetting(int exp, int afe, int isp);

        void setFlashOnAAOBuffer(void *buf);
        void* getFlashOnAAOBuffer();
    private:
        int mSensorDev;
        char m_FlashOnAAOBuffer[FLASH_AAO_BUF_SIZE];
};

template <class T>
int convertAaSttToY(void *buf, int w, int h, T *y,
        int gain, NS3Av3::EBitMode_T mode);

template <class T>
int convertAaSttToYrgb(void *buf, int w, int h, T *y, T *r, T *g, T *b,
        NS3Av3::EBitMode_T mode, int linearOutputEn = 0);
