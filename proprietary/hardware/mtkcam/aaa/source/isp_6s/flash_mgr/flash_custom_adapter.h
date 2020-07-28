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
#include "flash_mgr_m.h"
#include "flash_hal.h"

/* flash custom adapter */
class FlashCustomAdapter
{
    public:
        FlashCustomAdapter(int sensor);
        ~FlashCustomAdapter();

        /** init/uninit */
        int init();
        int uninit();

        static FlashCustomAdapter *getInstance(int sensor);

        /* init para */
        int getPfPolicy();
        int getDutyNum();
        int getDutyNumLt();
        int getFlashHalInfo(FlashHalInfo *pFlashHalInfo);

        /* precapture */
        int getFlashModeStyle(int sensorDev, int flashMode);
        int getPfEng();
        FLASH_AWB_ALGO_GAIN_N_T *getFlashWBGain();

        /* for panel flash*/
        int getDecideMfDutyIdx();

        /* get available duty range*/
        int getMfDutyMax(void);
        int getMfDutyMaxLt();

        int getMfDutyMin(FlashCapInfo *pCapInfo);
        int getMfDutyMax(FlashCapInfo *pCapInfo);
        int getMfDutyMinLt(FlashCapInfo *pCapInfo);
        int getMfDutyMaxLt(FlashCapInfo *pCapInfo);
        int getPfDuty(FlashCapInfo *pCapInfo);
        int getPfDutyLt(FlashCapInfo *pCapInfo);

        int getFlashMaxIDuty(int *duty, int *dutyLt);
        int isValidDuty(int duty, int dutyLt);

        /* get yTab*/
        int getYTabByDutyIndex(int dutyIndex);
        int setYTabByDutyIndex(int dutyIndex, int y);
        int setWBGainByDutyIndex(int dutyIndex, double Vr, double Vg, double Vb);
        int getWBRGainByDutyIndex(int dutyIndex);
        int getWBGGainByDutyIndex(int dutyIndex);
        int getWBBGainByDutyIndex(int dutyIndex);
        int clearYTab();

        /* awb tuning */
        int showAWBTuningPara();
        int flashAWBInit(FLASH_AWB_INIT_T *pFlashAwbInitParam);
        int getAWBRotationMatrix(FlashAlgInputInfoStruct *pFlashAlgoInfo);

        /* misc */
        int isVBatLow(FlashCapInfo *pCapInfo);
        int dumpNvEngTab();
        int dumpNvFlashAe(const char *fname);
        int dumpNvFlashAwb(const char *fname, int len);

        int getFastKExp(int *pExpBase, int *pAfeBase, int *pIspBase);
        /**
         * @brief Get flash project parameters.
         *
         * Flash project parameters is classified by AE scene and force flash
         *
         * @param[in] aeScene: AE scene (defined in AEPlinetable.h)
         * @param[in] isForceFlash: is force flash or not
         * @return flash project parameter
         */
        //TODO: move to private
        FLASH_PROJECT_PARA &getFlashProjectPara(int aeScene, int isForceFlash);

        /**
         * @brief Set NVRAM index for parameter mapping.
         * @param[in] eNVRAM: enum specifying parameter group
         * @param[in] a_eNVRAMIndex: mapping index for scenario
         */
        int setNVRAMIndex(FLASH_NVRAM_ENUM eNVRAM, MUINT32 a_eNVRAMIndex);

        int getFlashCaliNvIdx();

        int getFaceFlashSupport();

        void dumpProjectPara(FLASH_PROJECT_PARA *pp);

        int getTorchDuty(int level, int *duty, int *dutyLt);

        void transformFlashCaliData();

    private:
        void transformFlashCaliData(int srcSensorDev, int dstSensorDev);

        int mSensorDev;
        int mFacingSensor;

        /** nvram data */
        NVRAM_CAMERA_STROBE_STRUCT *mpStrobeNvbuf;
        NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT *mpFlashCaliNvbuf;
        FLASH_AWB_ALGO_TUNING_PARAM_T mFlashAwbTuning;
        FLASH_AWB_ALGO_GAIN_N_T mFlashWBGain[FLASH_CUSTOM_MAX_DUTY_NUM];

        int mFlashAENVRAMIdx;
        int mFlashAWBNVRAMIdx;
        int mFlashCaliNVRAMIdx;

        /** pre-capture */
        int mPfPolicy; /** pre-flash flow policy*/

        /** duty */
        int mDutyNum;
        int mDutyNumLT;

        short mITab1[FLASH_CUSTOM_MAX_DUTY_NUM];
        short mITab2[FLASH_CUSTOM_MAX_DUTY_NUM];
        CURRENT_MASK_FUNC mCurrentMaskFunc;
};

int dumpFlashAwb(const char *fname, FLASH_AWB_GAIN_N_T *gain, int len, int isDual, int dutyNum, int dutyNumLt);
void dumpFwbGain(FLASH_AWB_GAIN_N_T *flashWBGain, int dutyNum, int dutyNumLt);
