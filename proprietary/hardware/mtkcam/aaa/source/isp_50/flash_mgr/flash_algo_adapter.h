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

#ifdef __cplusplus
extern "C" {
#endif
#include "flash_ae_interface_control.h"
#include "flash_awb_interface_control.h"
#include "flash_cal_interface_control.h"
#ifdef __cplusplus
}
#endif

#include <dbg_aaa_param.h>
#include "dbg_flash_algo_param.h"

/* flash algo adapter */
class FlashAlgoAdapter
{
    public:
        FlashAlgoAdapter(int sensor);
        ~FlashAlgoAdapter();

        /** init/uninit */
        void init();
        void unInit();

        static FlashAlgoAdapter *getInstance(int sensor);

        int Estimate(FlashAlgExpPara* exp, FlashAlgFacePos* pFaceInfo, int* isLowRef);
        int setFlashType(int type);
        int Reset();
        int AddStaData10(FlashAlgStaData* pData, FlashAlgExpPara* pExp,
                int* pIsNeedNext, FlashAlgExpPara* pExpNext, int last);

        int CalFirstEquAEPara(FlashAlgExpPara* exp, FlashAlgExpPara* EquExp);
        int setFlashProfile(FlashAlgStrobeProfile* pr);
        int setCapturePLine(PLine* p, int isoAtGain1x);
        int setPreflashPLine(PLine* p, int isoAtGain1x);

        int setEVCompEx(float ev_comp, float tar, float evLevel);

        int setDefaultPreferences();
        int setTuningPreferences(FLASH_TUNING_PARA *tuning_p, int lv);

        int setFlashInfo(FlashAlgInputInfoStruct *pFlashInfo);

        int Flash_Awb_Init(FLASH_AWB_INIT_T &FlashAwbInit);
        int Flash_Awb_Algo(int Exp, int Iso, MUINT32 *FlashResultWeight);
        MRESULT DecideCalFlashComb(int CalNum, NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT *pNvCali,
                int totalStep, int totalStepLT, ChooseResult *pChoose,
                int dualFlashEnable, int caliNvIdx);

        MRESULT InterpolateCalData(int CalNum, short *dutyI, short *dutyLtI,
                CalData *pCalData, NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT *pNvCali,
                int totalStep, int totalStepLT, AWB_GAIN_T *outWB, short *outYTab,
                int dualFlashEnable, int CaliNvIdx);

        int setIsSaveSimBinFile(int isSaveBin);
        int setDebugDir(const char *DirName, const char *PrjName);
        int getDbgData(FLASH_DEBUG_INFO_T *pDebugInfo);
        int checkInputParaError(int* num, int* errBuf);

    private:
        inline void setDebugTag(FLASH_DEBUG_INFO_T &a_rFlashInfo, MINT32 a_i4ID, MINT32 a_i4Value)
        {
            a_rFlashInfo.Tag[a_i4ID].u4FieldID = AAATAG(AAA_DEBUG_FLASH_MODULE_ID, a_i4ID, 0);
            a_rFlashInfo.Tag[a_i4ID].u4FieldValue = a_i4Value;
        }

        int mSensorDev;
        int mSensorAlgo;
};

