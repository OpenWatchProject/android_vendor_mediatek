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
#include "mtkcam/aaa/aaa_hal_common.h"
#include "dbg_aaa_param.h"
#include "camera_custom_nvram.h"

enum
{
    e_SpModeNormal,
    e_SpModeUserExp,
    e_SpModeCalibration,
    e_SpModeQuickCalibration,
    e_SpModeQuickCalibration2,
};

/** panel flash status */
enum
{
    e_noflash,
    e_preflash,
    e_mainflash,
};

/** flash nvram enum */
typedef enum {
    FLASH_NVRAM_AE,
    FLASH_NVRAM_AWB,
    FLASH_NVRAM_CALIBRATION,
    FLASH_NVRAM_NUM,
} FLASH_NVRAM_ENUM;

/**
 * It's the interface with 3A flow for input data structure.
 * Only used by "doPfOneFrame()" currently.
 */
typedef struct {
    void *staBuf;
    int staX;
    int staY;
    int flickerMode;
    AWB_GAIN_T NonPreferencePrvAWBGain;
    AWB_GAIN_T PreferencePrvAWBGain;
    MINT32 i4AWBCCT;
} FlashExePara;

/**
 * It's the interface with 3A flow for output data structure.
 * Only used by "doPfOneFrame()" currently.
 */
typedef struct {
    int nextIso;
    int nextIsFlash;
    int nextExpTime;
    int nextAfeGain;
    int nextIspGain;
    int nextDuty;
    int isEnd;
    int isCurFlashOn;
    int flashAwbWeight;
    int isFlashCalibrationMode;
} FlashExeRep;

class FlashMgr
{
    protected:
        virtual ~FlashMgr() = 0;

    public:
        static FlashMgr *getInstance(int sensorDev);

        virtual int init() = 0;
        virtual int uninit() = 0;
        virtual int start() = 0;
        virtual int stop() = 0;

        virtual int getInCharge() = 0;
        virtual int setInCharge(int inCharge) = 0;

        virtual int getFlashSpMode() = 0;
        virtual int getFlashMode() = 0;
        virtual int getFlashFlowType() = 0;
        virtual int setAeFlashMode(int aeMode, int flashMode) = 0;
        virtual int getAfLampMode() = 0;
        virtual int setCamMode(int mode) = 0;
        virtual int setBitMode(NS3Av3::EBitMode_T mode) = 0;
        virtual int setDigZoom(int digx100) = 0;
        virtual int setEvComp(int index, float evStep) = 0;
        virtual int setManualExposureTime(MINT64 i8ExposureTime) = 0;
        virtual int setManualSensitivity(MINT32 i4Sensitivity) = 0;
        virtual int setMultiCapture(int multiCapture) = 0;
        virtual int setNVRAMIndex(FLASH_NVRAM_ENUM eNVRAM, MUINT32 a_eNVRAMIndex) = 0;

        virtual int isFlashOnCapture() = 0;
        virtual int setIsFlashOnCapture(int flashOnCapture) = 0;
        virtual int isFlashOnCalibration() = 0;

        virtual int updateFlashState() = 0;
        virtual int getFlashState() = 0;
        virtual int doPfOneFrame(FlashExePara *para, FlashExeRep *rep) = 0;
        virtual int endPrecapture() = 0;
        virtual int setCapPara() = 0;
        virtual int setAlgoExpPara(int exp, int afe, int isp, int isFlashOn) = 0;
        virtual int getDebugInfo(FLASH_DEBUG_INFO_T *p) = 0;
        virtual int setSubFlashState(int state) = 0;
        virtual int getSubFlashState() = 0;

        virtual int cctSetSpModeLock(int lock) = 0;
        virtual int cctSetSpModeCalibration() = 0;
        virtual MBOOL cctIsSpModeCalibration() = MFALSE;
        virtual int cctSetSpModeQuickCalibration2() = 0;
        virtual int cctSetSpModeNormal() = 0;
        virtual int cctGetQuickCalibrationResult() = 0;
        virtual int cctFlashEnable(int enable) = 0;
        virtual int cctFlashLightTest(void *pIn) = 0;
        virtual int cctGetFlashInfo(int *isOn) = 0;
        virtual int cctWriteNvram() = 0;
        virtual int cctReadNvramToPcMeta(void *out, MUINT32 *realOutSize) = 0;
        virtual int cctSetNvdataMeta(void *in, int inSize) = 0;
        virtual int setManualFlash(int duty, int dutyLt) = 0;
        virtual int clearManualFlash() = 0;
        virtual int dumpMainFlashAAO(void* aao);
        virtual int isHighBrightnessFlow() = 0;
        virtual void setPfHighBrightnessFlow(int isHighBrightness) = 0;
        virtual void setMVHDRMode(int &i4Mode) = 0;
        virtual int setFDInfo(void *a_sFaces, int i4tgwidth, int i4tgheight) = 0;
        virtual void setFlashOnAAOBuffer(void *buf) = 0;
        virtual void* getFlashOnAAOBuffer() = 0;
};

