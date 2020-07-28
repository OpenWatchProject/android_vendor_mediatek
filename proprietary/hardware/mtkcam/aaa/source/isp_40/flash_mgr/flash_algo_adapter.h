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

#include "mtkcam/def/BuiltinTypes.h"
#include "flash_param.h" // needed by FlashAlg.h
#include "flash_awb_param.h" // needed by FlashAlg.h
#include "FlashAlg.h"

/* flash algo adapter */
class FlashAlgoAdapter
{
    public:
        FlashAlgoAdapter(int sensor);
        ~FlashAlgoAdapter();

        /** init/uninit */
        int init();
        int unInit();

        static FlashAlgoAdapter *getInstance(int sensor);

        int Estimate(NS3A::FlashAlgExpPara* exp, NS3A::FlashAlgFacePos* pFaceInfo, int* isLowRef);
        int setFlashType(int type);
        int Reset();
        int AddStaData10(NS3A::FlashAlgStaData* pData, NS3A::FlashAlgExpPara* pExp,
                int* pIsNeedNext, NS3A::FlashAlgExpPara* pExpNext, bool last);

        int CalFirstEquAEPara(NS3A::FlashAlgExpPara* exp, NS3A::FlashAlgExpPara* EquExp);
        int setFlashProfile(NS3A::FlashAlgStrobeProfile* pr);
        int setCapturePLine(NS3A::PLine* p, int isoAtGain1x);
        int setPreflashPLine(NS3A::PLine* p, int isoAtGain1x);

        int setEVCompEx(float ev_comp, float tar, float evLevel);

        int setDefaultPreferences();
        int setTuningPreferences(FLASH_TUNING_PARA *tuning_p, int lv);

        int setFlashInfo(NS3A::FlashAlgInputInfoStruct *pFlashInfo);

        int Flash_Awb_Init(FLASH_AWB_INIT_T &FlashAwbInit);
        int Flash_Awb_Algo(int Exp, int Iso, MUINT32 *FlashResultWeight);
        MRESULT DecideCalFlashComb(int CalNum, NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT *pNvCali,
                int totalStep, int totalStepLT, NS3A::ChooseResult *pChoose,
                bool dualFlashEnable, int caliNvIdx);

        MRESULT InterpolateCalData(int CalNum, short *dutyI, short *dutyLtI,
                NS3A::CalData *pCalData, NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT *pNvCali,
                int totalStep, int totalStepLT, AWB_GAIN_T *outWB, short *outYTab,
                bool dualFlashEnable, int CaliNvIdx);

        int setIsSaveSimBinFile(int isSaveBin);
        int setDebugDir(const char *DirName, const char *PrjName);
        int getDbgData(FLASH_DEBUG_INFO_T *pDebugInfo);
        int checkInputParaError(int* num, int* errBuf);

    private:
        /** flash algo handler */
        NS3A::FlashAlg *mpFlashAlg;
};

