/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein is
* confidential and proprietary to MediaTek Inc. and/or its licensors. Without
* the prior written permission of MediaTek inc. and/or its licensors, any
* reproduction, modification, use or disclosure of MediaTek Software, and
* information contained herein, in whole or in part, shall be strictly
* prohibited.
*
* MediaTek Inc. (C) 2010. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
* ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
* WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
* NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
* RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
* INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
* TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
* RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
* OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
* SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
* RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
* ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
* RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
* MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
* CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* The following software/firmware and/or related documentation ("MediaTek
* Software") have been modified by MediaTek Inc. All revisions are subject to
* any receiver's applicable license agreements with MediaTek Inc.
*/

/**
* @file IResultPool.h
* @brief Declarations of Interfance of Result Pool Manager
*/
#ifndef __IRESULT_POOL_H__
#define __IRESULT_POOL_H__

#include <string>
#include <utils/Mutex.h>
#include <utils/Vector.h>
#include <semaphore.h>

#include <mtkcam/aaa/IHal3A.h>

#include <camera_custom_nvram.h>
#include <isp_tuning_cam_info.h>

// 3A modules Entry
#include <Entry.h>
#include <ResultPool4Module.h>

using namespace android;
using namespace NSCam;

namespace NS3Av3
{

/*********************************************************
 *
 *   ResultPool define for 3A HAL
 *
 *********************************************************/
#define SubsampleCount (1)
#define HistorySize (3)
#define ConvertNum4SMVR (4)
#define InvalidValue (-1)
// Because W+T slave set faster than get for getCur
#if 1//MTK_SUPPORT_ENG
#define ResultNum (16)//Align MW result number
#else
#define ResultNum (8)//Align MW result number
#endif
#define ResultNum4Dummy (1) //dummy buffer
#define ResultDummyIndex (8) //dummy buffer index
#define Capacity (ResultNum+ResultNum4Dummy)

/*********************************************************
 *
 *   Define 3A convert type
 *
 *********************************************************/
typedef enum
{
    E_NON_CONVERT   = 0,
    E_2A_CONVERT    = (1<<0),
    E_AF_CONVERT    = (1<<1)
} E_TYPY_CONVERT;

/*********************************************************
 *
 *   Define 3A EXIF info
 *
 *********************************************************/
struct EXIF_3A_INFO_T
{
    MUINT32     u4FNumber;                  // Format: F2.8 = 28
    MUINT32     u4FocalLength;              // Format: FL 3.5 = 350
    MUINT32     u4AWBMode;                  // White balance mode          update!!
    MUINT32     u4LightSource;              // Light Source mode             new!!
    MUINT32     u4ExpProgram;               // Exposure Program              new!!
    MUINT32     u4SceneCapType;             // Scene Capture Type            new!!
    MUINT32     u4FlashLightTimeus;         // Strobe on/off
    MUINT32     u4AEMeterMode;              // Exposure metering mode   update!!
    MINT32      i4AEExpBias;                // Exposure index*10
    MUINT32     u4CapExposureTime;          //
    MUINT32     u4AEISOSpeed;               // AE ISO value                       update!!
    MUINT32     u4SceneMode;                // Scene mode   (SCENE_MODE_XXX)
    MUINT32     u4RealISOValue;             // ISO value

    EXIF_3A_INFO_T()  { ::memset(this, 0, sizeof(EXIF_3A_INFO_T)); }
};

/*********************************************************
 *
 *   Define 3A slow motion info. This is for slow motion.
 *
 *********************************************************/
struct SMVR_GAIN_DELAY_INFO_T {
    MUINT32 u4SensorGain_Delay;
    MUINT32 u4SutterGain_Delay;
    MUINT32 u4ISPGain_Delay;
};

struct SLOW_MOTION_AF_INFO_T
{
    MUINT8              u1AfState;
    SLOW_MOTION_AF_INFO_T()  { ::memset(this, 0, sizeof(SLOW_MOTION_AF_INFO_T)); }
};

struct SLOW_MOTION_3A_INFO_T
{
    MINT32                  i4ConvertMagic[ConvertNum4SMVR]; // Need to convert Magic Number
    MUINT8                  u1AeState;
    MINT64                  i8SensorExposureTime;
    MINT32                  i4SensorSensitivity;
    SMVR_GAIN_DELAY_INFO_T  rGainDelay;
    SLOW_MOTION_AF_INFO_T   rAfInfo;
    SLOW_MOTION_3A_INFO_T()  { ::memset(this, 0, sizeof(SLOW_MOTION_3A_INFO_T)); }
};

/*********************************************************
 *
 *   Define 3A resultPool basic config info.
 *
 *********************************************************/
struct RESULT_CFG_T
{
    MINT32                  i4ReqMagic;           // For giving request magicNum from MW. i4ReqMagic is equal i4HistoryReqMagic2
    MINT32                  i4StatisticMagic;     // For giving statistic magicNum from AAOBufMgr. It gives value in Hal3AFlowCtrl::postCommand before call on3AProcFinish.
    std::string             strUserName;          // For debug info, modules update result.

    RESULT_CFG_T()
        : i4ReqMagic(0)
        , i4StatisticMagic(InvalidValue)
        , strUserName("")
        {}
};

/*********************************************************
 *
 *   Define 3A early callBack. This is for ZSD. Because MW need to know AfState for flash.
 *
 *********************************************************/
struct EARLY_CALL_BACK
{
    MUINT8      u1AfState;
    MUINT8      u1AeState;
    MUINT8      u1AeMode;
    MUINT8      u1FlashState;
    MBOOL       fgAeBvTrigger;
    MINT32      i4MasterIdx;
    MINT32      i4SlaveIdx;

    EARLY_CALL_BACK()
        : u1AfState(0)
        , u1AeState(0)
        , u1AeMode(0)
        , u1FlashState(0)
        , fgAeBvTrigger(0)
        , i4MasterIdx(0)
        , i4SlaveIdx(0)
        {}
};

/*********************************************************
 *
 *   Define 3A ResultPool Sensor Info.
 *
 *********************************************************/
struct BASIC_CFG_INFO_T
{
    MINT32                  i4SensorIdx;
    MUINT32                 u4SensorDev;
    MINT32                  i4SensorMode;
    BASIC_CFG_INFO_T()
        : i4SensorIdx(0)
        , u4SensorDev(0)
        , i4SensorMode(0)
    {}
};

/*********************************************************
 *
 *   Define 3A Important info. Keep Important module info, because print log use
 *
 *********************************************************/
struct IMPORTANT_INFO_T
{
    MUINT8      u1AeState;
    MUINT8      u1AfState;
    MUINT8      u1AwbState;
    MUINT8      u1FlashState;
    MINT64      i8SensorExposureTime;
    MINT64      i8SensorFrameDuration;
    MINT32      i4SensorSensitivity;
    MUINT8      u1CapIntent;
    MUINT8      u1AePrecapTrig;
    MUINT8      u1AfTrig;
    MUINT8      u1AWBMode;
    MUINT8      u1ColorCorrectMode;
    IMPORTANT_INFO_T()
        : u1AeState(0)
        , u1AfState(0)
        , u1AwbState(0)
        , u1FlashState(0)
        , i8SensorExposureTime(0)
        , i8SensorFrameDuration(0)
        , i4SensorSensitivity(0)
        , u1CapIntent(0)
        , u1AePrecapTrig(0)
        , u1AfTrig(0)
        , u1AWBMode(1)
        , u1ColorCorrectMode(1)
        {}
};

/*********************************************************
 *
 *   Define 3A convert result to metadata command
 *
 *********************************************************/
struct ResultPoolCmd4Convert_T
{
    MINT32                  i4MagicNumReq;
    MINT32                  i4SubsampleCount;
    E_TYPY_CONVERT          eConvertType;
    ResultPoolCmd4Convert_T()
        : i4MagicNumReq(0)
        , i4SubsampleCount(0)
        , eConvertType(E_2A_CONVERT)
        {}

    ResultPoolCmd4Convert_T(MINT32 _i4Magic, MINT32 _i4SubsampleCount, E_TYPY_CONVERT _eConvertType)
        : i4MagicNumReq(_i4Magic)
        , i4SubsampleCount(_i4SubsampleCount)
        , eConvertType(_eConvertType)
        {}
};

/*********************************************************
 *
 *   Define 3A ResultPool Info.
 *
 *********************************************************/
struct AllResult_T
{
    Entry*                  ModuleResultAddr[static_cast<MINT32>(E_NUM_RESULT_END)];

    RESULT_CFG_T            rResultCfg;
    SLOW_MOTION_3A_INFO_T   rOld3AInfo;// For slow motion
    EARLY_CALL_BACK         rEarlyCB;
    Mutex                   LockVecResult;

    // exif
    android::Vector<EXIF_3A_INFO_T> vecExifInfo;
    android::Vector<MUINT8> vecDbg3AInfo;
    android::Vector<MUINT8> vecDbgShadTbl;
    android::Vector<MUINT8> vecDbgIspInfo;
    android::Vector<MUINT8> vecDbgN3DInfo;
    // AE AF AWB ROI for APP META
    android::Vector<MINT32> vecAEROI;
    android::Vector<MINT32> vecAFROI;
    android::Vector<MINT32> vecAWBROI;
    // focus area result
    android::Vector<MINT32> vecFocusAreaPos;
    android::Vector<MUINT8> vecFocusAreaRes;
    // ISP
    android::Vector<MFLOAT> vecColorCorrectMat;
    // tonemap => slow motion in pass1, normal in pass 2
    android::Vector<MFLOAT> vecTonemapCurveBlue;
    android::Vector<MFLOAT> vecTonemapCurveGreen;
    android::Vector<MFLOAT> vecTonemapCurveRed;
    // LSC
    android::Vector<MUINT8> vecLscData;

    AllResult_T()
        : LockVecResult()
    {
        ::memset( ModuleResultAddr, NULL, sizeof(Entry *)*(static_cast<MINT32>(E_NUM_RESULT_END)));
    }
};

/*********************************************************
 *
 *   Define 3A Metadata ResultPool Info.
 *
 *********************************************************/
struct AllMetaResult_T
{
    Mutex LockMetaResult;
    MetaSet_T rMetaResult;
    MINT32    i4IsConverted;

    AllMetaResult_T()
        : LockMetaResult()
        , rMetaResult()
        , i4IsConverted(E_NON_CONVERT)
    {}
};

/*********************************************************
 *   Result to Meta structure
 *   xxxResultToMeta_T is for metadata use.
 *   If modules member need to update to metadata. Please add here.
 *********************************************************/
struct HALResultToMeta_T
{
    MINT32      i4FrmId;
    MBOOL       fgKeep;
    MBOOL       fgBadPicture;
    MUINT8      u1SceneMode;
    // stereo warning message
    MINT32      i4StereoWarning;

    HALResultToMeta_T()
        : i4FrmId(-1)
        , fgKeep(0)
        , fgBadPicture(0)
        , u1SceneMode(0)
        , i4StereoWarning(0)
    {}
};

struct AEResultToMeta_T
{
    MUINT8      u1AeState;
    MUINT8      u1AeMode;
    MBOOL       fgAeBvTrigger;
    MINT32      i4AutoHdrResult;
    // sensor
    MINT64      i8SensorRollingShutterSkew;
    MINT64      i8SensorExposureTime;
    MINT64      i8SensorFrameDuration;
    MINT32      i4SensorSensitivity;
    MINT32      i4AvgY;
    MINT32      i4SensorGain;
    MINT32      i4IspGain;
    MINT32      i4LuxIndex;

    AEResultToMeta_T()
        : u1AeState(0)
        , u1AeMode(0)
        , fgAeBvTrigger(0)
        , i4AutoHdrResult(0)
        , i8SensorRollingShutterSkew(0)
        , i8SensorExposureTime(0)
        , i8SensorFrameDuration(0)
        , i4SensorSensitivity(0)
        , i4AvgY(0)
        , i4SensorGain(0)
        , i4IspGain(0)
        , i4LuxIndex(0)
    {}
};

struct AFResultToMeta_T
{
    MUINT8      u1AfState;
    MUINT8      u1LensState;
    // lens
    MFLOAT      fLensAperture;
    MFLOAT      fLensFilterDensity;
    MFLOAT      fLensFocalLength;
    MFLOAT      fLensFocusDistance;
    MFLOAT      fLensFocusRange[2];
    // focus area result
    MINT32      i4FocusAreaSz[2];

    AFResultToMeta_T()
        : u1AfState(0)
        , u1LensState(0)
        , fLensAperture(0)
        , fLensFilterDensity(0)
        , fLensFocalLength(0)
        , fLensFocusDistance(0)
    {
        ::memset( &fLensFocusRange, 0, sizeof(MFLOAT)*2);
        ::memset( &i4FocusAreaSz, 0, sizeof(MINT32)*2);
    }
};

struct AWBResultToMeta_T
{
    MUINT8      u1AwbState;
    // color correction
    MINT32      i4AwbGain[3];
    MINT32      i4AwbGainScaleUint;
    MFLOAT      fColorCorrectGain[4];
    MINT32      i4MWBColorTemperatureMax;
    MINT32      i4MWBColorTemperatureMin;
    MINT32      i4AwbAvailableRange[2];

    AWBResultToMeta_T()
        : u1AwbState(0)
        , i4AwbGainScaleUint(0)
        , i4MWBColorTemperatureMax(0)
        , i4MWBColorTemperatureMin(0)
    {
        ::memset( &i4AwbGain, 0, sizeof(MINT32)*3);
        ::memset( &fColorCorrectGain, 0, sizeof(MFLOAT)*4);
        ::memset( &i4AwbAvailableRange, 0, sizeof(MINT32)*2);
    }
};

struct ISPResultToMeta_T
{
    // cam info
    NSIspTuning::RAWIspCamInfo rCamInfo;
    MINT32 i4LceGain;
    ISPResultToMeta_T()
        : i4LceGain(0)
    {}
};

struct LSCResultToMeta_T
{
    // shading map
    MBOOL       fgShadingMapOn;
    MINT32      i4CurrTblIndex;

    LSCResultToMeta_T()
        : fgShadingMapOn(0)
        , i4CurrTblIndex(0)
    {}
};

struct FLASHResultToMeta_T
{
    MUINT8      u1FlashState;
    MBOOL       bCustEnableFlash; // during touch
    MINT32      i4FlashCalResult;    // flash calibration result
    MUINT8      u1SubFlashState;
    MUINT32     u4StrobeMode;
    MINT32      i4FlashScenario;

    FLASHResultToMeta_T()
        : u1FlashState(0)
        , bCustEnableFlash(0)
        , i4FlashCalResult(0)
        , u1SubFlashState(0)
        , u4StrobeMode(0)
        , i4FlashScenario(-1)
    {}
};

struct FLKResultToMeta_T
{
    MUINT8      u1SceneFlk;

    FLKResultToMeta_T()
        : u1SceneFlk(0)
    {}
};

struct Sync3AResultToMeta_T
{
    DualZoomInfo_T rDualZoomInfo;

    Sync3AResultToMeta_T()
    {}
};

/*********************************************************
 *
 *   Last information structure.
 *
 *********************************************************/
struct LastInfo_T
{
    NSIspTuning::RAWIspCamInfo      mBackupCamInfo;
    MBOOL                           mBackupCamInfo_copied;
};

/*********************************************************
 *
 *   Modules need to add case for new structure.
 *
 *********************************************************/
inline MINT32 enumToSize(E_PARTIAL_RESULT_OF_MODULE_T ModuleEnum)
{
    switch (ModuleEnum)
    {
        case E_HAL_RESULTTOMETA:
            return sizeof(HALResultToMeta_T);
        case E_AE_RESULTTOMETA:
            return sizeof(AEResultToMeta_T);
        case E_AF_RESULTTOMETA:
            return sizeof(AFResultToMeta_T);
        case E_AWB_RESULTTOMETA:
            return sizeof(AWBResultToMeta_T);
        case E_ISP_RESULTTOMETA:
            return sizeof(ISPResultToMeta_T);
        case E_LSC_RESULTTOMETA:
            return sizeof(LSCResultToMeta_T);
        case E_FLASH_RESULTTOMETA:
            return sizeof(FLASHResultToMeta_T);
        case E_FLK_RESULTTOMETA:
            return sizeof(FLKResultToMeta_T);
        case E_SYNC3A_RESULTTOMETA:
            return sizeof(Sync3AResultToMeta_T);
        case E_NUM_RESULTTOMETA:
            return -1;
        case E_NUM_RESULT_END:
            return -1;
    }
}

/*********************************************************
 *
 *   Interface of IResultPool
 *
 *********************************************************/
class IResultPool
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    IResultPool(){}
    virtual ~IResultPool(){}
    // copy constructor and copy assignment operator
    IResultPool(const IResultPool&);
    IResultPool& operator=(const IResultPool&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces for 3A HAL / Modules.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief User want to use resultPool need to call getInstance
     */
    static IResultPool*             getInstance(MINT32 i4SensorDevId);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces for moduls.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief get partial of 3A result
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [in]  eModule is 3A enum; please refer to E_PARTIAL_RESULT_OF_MODULE_T
     * @param [out] return result which is partial of 3A result. Or return NULL.
     */
    virtual const MVOID*            getResult(MINT32 i4ReqMagic, E_PARTIAL_RESULT_OF_MODULE_T eModule) = 0;

    /**
     * @brief update partial of 3A result
     * @param [in]  strUserName is which module update; It refer to LOG_TAG.
     * @param [in]  i4ReqMagic is current request; It refer to magic number of set function
     * @param [in]  eModule is 3A enum; please refer to E_PARTIAL_RESULT_OF_MODULE_T
     * @param [in]  pData is partial of 3A result which module update.
     */
    virtual MINT32                  updateResult(std::string strUserName, MINT32 i4ReqMagic, E_PARTIAL_RESULT_OF_MODULE_T eModule, const MVOID* pData) = 0;
    /**
     * @brief get PSO info of cameInfo and LSC.
     * @param [in]  u4GridPointNum definition by Ae_mgr.
     * @param [in]  i4SttMagic is statistic magic number;
     * @param [out] rPSOInfo has PSO info.
     */
#if (!CAM3_3A_ISP_30_EN)
    virtual MBOOL                   getAAOInfo(MUINT32 u4GridPointNum, MINT32 i4SttMagic, AAO_PROC_INFO_T& rPSOInfo) = 0;
    virtual MBOOL                   getAAOInfoTest(MUINT32 u4GridPointNum, MINT32 i4SttMagic, AAO_PROC_INFO_T& rPSOInfo)= 0;
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces for 3A HAL.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief Push basic convert to meta enum to list and Call allocateResultPool()
     */
    virtual MVOID                   init() = 0;

    /**
     * @brief Clear list and Call freeResultPool()
     */
    virtual MVOID                   uninit() = 0;

    /**
     * @brief Update SensorIdx, SensorDev, SensorMode to local member.
     * @param [in]  rBasicCfg is sensor basic info.
     */
    virtual MVOID                   config(const BASIC_CFG_INFO_T& rBasicCfg) = 0;

    /**
     * @brief 3A update Capacity or not
     * @param [in]  u4Capacity is size of result buffer pool.
     * @param [in]  i4SubsampleCount is for slow motion.
     */
    virtual MINT32                  updateCapacity(MUINT32 u4Capacity, MINT32 i4SubsampleCount) = 0;

    /**
     * @brief get config of result
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return reference CFG;
     */
    virtual RESULT_CFG_T*           getResultCFG(MINT32 i4ReqMagic) = 0;

    /**
     * @brief set Meta Static Info to resultPool
     * @param [in]  metaStaticInfo is meta info
     */
    virtual MVOID                   setMetaStaticInfo(const IMetadata& metaStaticInfo) = 0;

    /**
     * @brief get Meta Static Info to resultPool
     * @param [out]  metaStaticInfo is meta info
     */
    virtual IMetadata&              getMetaStaticInfo() = 0;

    /**
     * @brief update magic number history to resultPool
     * @param [in] i4ReqMagic is which you want request magic number.
     * @param [in] Size is buffer size.
     */
    virtual MVOID                   updateHistory(MINT32 i4ReqMagic, MINT32 Size) = 0;

    /**
     * @brief get magic number history from resultPool
     * @param [out] pHistory is buffer.
     */
    virtual MINT32                  getHistory(MINT32* pHistory) = 0;

    /**
     * @brief update config of result
     * @param [in]  i4ReqMagic is which you want request magic number, i4SttMagic is statistic magic number;
     * @param [out] return false or true;
     */
    virtual MBOOL                   updateResultCFG(MINT32 i4ReqMagic, MINT32 i4SttMagic) = 0;

    /**
     * @brief Clear all result pool.
     */
    virtual MVOID                   clearAllResultPool() = 0;

    /**
     * @brief get all of 3A result
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return result which is all of 3A result. Or return NULL.
     */
    virtual AllResult_T*            getAllResult(MINT32 i4ReqMagic) = 0;

    /**
     * @brief get all of 3A result
     * @param [in]  i4SttMagic is which you want statistic magic number;
     * @param [out] return result which is all of 3A result. Or return NULL.
     */
    virtual AllResult_T*            getAllResultCur(MINT32 i4SttMagic) = 0;

    /**
     * @brief get all of 3A result. Avoid getCur function getting fail.
     *  Use i4ReqMagic 0 to get.
     */
    virtual AllResult_T*            getAllResultLastCur(MINT32 i4ReqMagic) = 0;

    /**
     * @brief update all of 3A result
     * @param [in]  strUserName is which module update; It refer to LOG_TAG.
     * @param [in]  i4ReqMagic is which you want to update request magic number;
     * @param [in]  pData is all of 3A result
     */
    virtual MINT32                  updateAllResult(std::string strUserName, MINT32 i4ReqMagic, const AllResult_T* pData) = 0;

    /**
     * @brief get metadata result
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return metadata result. Or return default metadata result.
     */
    virtual AllMetaResult_T*        getMetadata(MINT32 i4ReqMagic) = 0;

    /**
     * @brief get validate metadata result buffer
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return metadata result buffer.
     */
    virtual MetaSet_T*              getValidateMetadata(MINT32 i4ReqMagic) = 0;

    /**
     * @brief If result is validate, then prepare result. For preview
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return false or true
     */
    virtual MINT32                  isValidate(MINT32 i4ReqMagic) = 0;

    /**
     * @brief If result is validate, then prepare result For HQC
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return false or true
     */
    virtual MINT32                  isValidateCur(MINT32 i4SttMagic) = 0;

    /**
     * @brief update module state of early callBack.
     * @param [in]  i4ReqMagic is which you want request magic number.
     * @param [in]  rEarlyCB is which you want to update struct.
     * @param [out] return false or true;
     */
    virtual MBOOL                   updateEarlyCB(MINT32 i4ReqMagic, EARLY_CALL_BACK &rEarlyCB) = 0;

    /**
     * @brief update module state of early callBack.
     * @param [in]  i4ReqMagic is which you want request magic number.
     * @param [out] return reference EARLY_CALL_BACK;
     */
    virtual EARLY_CALL_BACK*        getEarlyCB(MINT32 i4ReqMagic) = 0;

    /**
     * @brief convert result to metadata for 2A
     * @param [in]  pAllResult is which you want to convert result;
     * @param [in]  pMetaResult is which you want to update metadata;
     */
    virtual MINT32                  convertToMetadata(AllResult_T* pAllResult, AllMetaResult_T* pMetaResult) = 0;

    /**
     * @brief convert result to metadata for AF
     * @param [in]  pAllResult is which you want to convert result;
     * @param [in]  pMetaResult is which you want to update metadata;
     */
    virtual MINT32                  convertToMetadata4AF(AllResult_T* pAllResult, AllMetaResult_T* pMetaResult) = 0;

    /**
     * @brief post to threadResultPool
     * @param [in] pCmd is resultPool command.
     */
    virtual MVOID                   postCmdToThread(ResultPoolCmd4Convert_T* pCmd) = 0;

    /**
     * @brief update LastInfo to result Pool
     * @param [in] LastInfo is camerainfo.
     */
    virtual MVOID                   updateLastInfo(LastInfo_T& vLastInfo) = 0;

    /**
     * @brief get LastInfo to result Pool.
     * @Need to call lockLastInfo before call getLastInfo. Then call unlockLastInfo after modify succcessful.
     * @param [out] return last info.
     */
    virtual LastInfo_T&             getLastInfo() = 0;

    /**
     * @brief get mutex for lock LastInfo buffer from result Pool
     * @param [out] return true.
     */
    virtual MBOOL                   lockLastInfo() = 0;

    /**
     * @brief get mutex for unlock LastInfo buffer from result Pool
     * @param [out] return true.
     */
    virtual MBOOL                   unlockLastInfo() = 0;

}; //IResultPool

} //namespace NS3Av3

#endif  //__IRESULT_POOL_H__
