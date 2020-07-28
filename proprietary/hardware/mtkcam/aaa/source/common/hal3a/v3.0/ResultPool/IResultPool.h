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
#include <mutex>
#include <vector>
#include <list>
#include <semaphore.h>

#include <mtkcam/aaa/IHal3A.h>

#include <camera_custom_nvram.h>
#include <isp_tuning_cam_info.h>

// 3A modules Entry
#include <Entry.h>

//#include <ResultPool4Module.h>
#include <ResultPoolBase.h>
#include <ResultPool4Meta.h>

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
#define ResultNum (16)        //Align MW result number
#else
#define ResultNum (8)        //Align MW result number
#endif
#define ResultNum4Dummy (1)  //dummy buffer
#define ResultDummyIndex (8) //dummy buffer index
#define Capacity (ResultNum+ResultNum4Dummy)
#define ConfigMagic (1)

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
    SMVR_GAIN_DELAY_INFO_T()  { ::memset(this, 0, sizeof(SMVR_GAIN_DELAY_INFO_T)); }
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
struct REQUEST_INFO_T
{
    MINT32                  i4ReqMagic;           // For giving request magicNum from MW. i4ReqMagic is equal i4HistoryReqMagic2
    MINT32                  i4StatisticMagic;     // For giving statistic magicNum from AAOBufMgr. It gives value in Hal3AFlowCtrl::postCommand before call on3AProcFinish.
    MINT32                  i4PreSetKey;

    REQUEST_INFO_T()
        : i4ReqMagic(InvalidValue)
        , i4StatisticMagic(InvalidValue)
        , i4PreSetKey(InvalidValue)
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
        : i4MagicNumReq(InvalidValue)
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
    Entry*                  ModuleResult[static_cast<MINT32>(E_NUM_RESULT_END)];

    REQUEST_INFO_T          rRequestInfo;
    SLOW_MOTION_3A_INFO_T   rOld3AInfo;// For slow motion
    EARLY_CALL_BACK         rEarlyCB;
    std::mutex              LockVecResult;

    // exif
    std::vector<EXIF_3A_INFO_T> vecExifInfo;
    android::Vector<MUINT8> vecDbg3AInfo;
    android::Vector<MUINT8> vecDbgShadTbl;
    android::Vector<MUINT8> vecDbgIspInfo;
    android::Vector<MUINT8> vecDbgN3DInfo;
    // AE AF AWB ROI for APP META
    std::vector<MINT32> vecAEROI;
    std::vector<MINT32> vecAFROI;
    std::vector<MINT32> vecAWBROI;
    // focus area result
    std::vector<MINT32> vecFocusAreaPos;
    std::vector<MUINT8> vecFocusAreaRes;
    // ISP
    std::vector<MFLOAT> vecColorCorrectMat;
    // tonemap => slow motion in pass1, normal in pass 2
    std::vector<MFLOAT> vecTonemapCurveBlue;
    std::vector<MFLOAT> vecTonemapCurveGreen;
    std::vector<MFLOAT> vecTonemapCurveRed;
    // LSC
    std::vector<MUINT8> vecLscData;

    AllResult_T()
        : LockVecResult()
    {
        ::memset( ModuleResult, NULL, sizeof(Entry *)*(static_cast<MINT32>(E_NUM_RESULT_END)));
    }
};

/*********************************************************
 *
 *   Define 3A Metadata ResultPool Info.
 *
 *********************************************************/
struct AllMetaResult_T
{
    std::mutex LockMetaResult;
    MetaSet_T rMetaResult;
    MINT32    i4IsConverted;

    AllMetaResult_T()
        : LockMetaResult()
        , rMetaResult()
        , i4IsConverted(E_NON_CONVERT)
    {}
};

struct P2Info_T
{
    NSCam::MRect rSclCropRectl;
    MINT32 AF_GMV_X;
    MINT32 AF_GMV_Y;
    MINT32 AF_GMV_Conf_X;
    MINT32 AF_GMV_Conf_Y;
    MINT32 AF_GMV_Max;
    P2Info_T()
        : rSclCropRectl()
        , AF_GMV_X(0)
        , AF_GMV_Y(0)
        , AF_GMV_Conf_X(0)
        , AF_GMV_Conf_Y(0)
        , AF_GMV_Max(0)
    {}
};

#if 0
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
        case E_NUM_RESULTTOMETA:
            return -1;
        case E_LCSO_RESULTTOMETA:
            return sizeof(LCSOResultToMeta_T);
        case E_LSC_CONFIGRESULTTOISP:
            return sizeof(LSCConfigResult_T);
        case E_NUM_RESULT_END:
            return -1;
    }
}
#endif
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
    virtual const MVOID*            getResult(const MINT32& i4ReqMagic, const E_PARTIAL_RESULT_OF_MODULE_T& eModule, const char* pFunctionName) = 0;

    /**
     * @brief update partial of 3A result
     * @param [in]  strUserName is which module update; It refer to LOG_TAG.
     * @param [in]  i4ReqMagic is current request; It refer to magic number of set function
     * @param [in]  eModule is 3A enum; please refer to E_PARTIAL_RESULT_OF_MODULE_T
     * @param [in]  pData is partial of 3A result which module update.
     */
    virtual MINT32                  updateResult(std::string strUserName, const MINT32& i4ReqMagic, const E_PARTIAL_RESULT_OF_MODULE_T& eModule, const MVOID* pData) = 0;

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
    virtual MINT32                  setCapacity(const MUINT32& u4Capacity, const MINT32& i4SubsampleCount) = 0;

    /**
     * @brief get information of request
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return CFG;
     */
    virtual REQUEST_INFO_T            getRequestInfo(const MINT32& i4ReqMagic) = 0;

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
    virtual MVOID                   setHistory(const MINT32& i4ReqMagic, const MINT32& Size) = 0;

    /**
     * @brief get magic number history from resultPool
     * @param [out] pHistory is buffer.
     */
    virtual MINT32                  getHistory(MINT32* pHistory) = 0;

    /**
     * @brief update information of request
     * @param [in]  i4ReqMagic is which you want request magic number, i4SttMagic is statistic magic number;
     * @param [out] return false or true;
     */
    virtual MBOOL                   setRequestInfo(const REQUEST_INFO_T& rRequestInfo) = 0;

    /**
     * @brief Clear all result pool.
     */
    virtual MVOID                   clearAllResultPool() = 0;

    /**
     * @brief get all of 3A result
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return result which is all of 3A result. Or return NULL.
     */
    virtual AllResult_T*            getResultByReqNum(const MINT32& i4ReqMagic, const char* pFunctionName) = 0;

    /**
     * @brief get all of 3A result
     * @param [in]  i4SttMagic is which you want statistic magic number;
     * @param [out] return result which is all of 3A result. Or return NULL.
     */
    virtual AllResult_T*            getResultBySttNum(const MINT32& i4SttMagic, const char* pFunctionName) = 0;

    /**
     * @brief get all of 3A result
     * @param [in]  i4PresetKey is which you want presetkey number;
     * @param [out] return result which is all of 3A result. Or return NULL.
     */
    virtual AllResult_T*            getResultByPresetKey(MINT32 i4PreSetKey, const char* pFunctionName) = 0;

    /**
     * @brief return 3A result to result pool
     * @param [in]  all of 3A result
     */
    virtual VOID                    returnResult(AllResult_T* pAllResult, const char* pFunctionName) = 0;

    /**
     * @brief get metadata result
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return metadata result. Or return default metadata result.
     */
    virtual AllMetaResult_T*        getMetadata(const MINT32& i4ReqMagic, const char* pFunctionName) = 0;

    /**
     * @brief return metadata to result pool
     * @param [in]  metadata
     */
    virtual VOID                    returnMetadata(AllMetaResult_T* pMetaResult, const char* pFunctionName) = 0;

    /**
     * @brief get validate result buffer
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return success or fail.
     */
    virtual MBOOL                   findValidateBuffer(MINT32 i4ReqMagic) = 0;

    /**
     * @brief If result is validate, then prepare result. For preview
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return false or true
     */
    virtual MINT32                  isValidateByReqNum(const MINT32& i4ReqMagic, const E_PARTIAL_RESULT_OF_MODULE_T& eModule) = 0;

    /**
     * @brief If result is validate, then prepare result For HQC
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return false or true
     */
    virtual MINT32                  isValidateBySttNum(const MINT32& i4SttMagic, const E_PARTIAL_RESULT_OF_MODULE_T& eModule) = 0;

    /**
     * @brief convert result to metadata for 2A
     * @param [in]  pAllResult is which you want to convert result;
     * @param [in]  pMetaResult is which you want to update metadata;
     */
    virtual MINT32                  convertToMetadata42A(AllResult_T* pAllResult, AllMetaResult_T* pMetaResult) = 0;

    /**
     * @brief convert result to metadata for AF
     * @param [in]  pAllResult is which you want to convert result;
     * @param [in]  pMetaResult is which you want to update metadata;
     */
    virtual MINT32                  convertToMetadata4AF(AllResult_T* pAllResult, AllMetaResult_T* pMetaResult) = 0;

    /**
     * @brief convert result to metadata flow
     * @param [in]  i4ReqMagic is which you want request magic number.
     * @param [in]  i4ConvertType is which you want convert type (2A or AF).
     * @param [in]  pAllResult is which you want to convert result;
     * @param [in]  pMetaResult is which you want to update metadata;
     */
    virtual MINT32                  convertToMetadataFlow(const MINT32& i4ReqMagic, MINT32 i4ConvertType, AllResult_T* pAllResult, AllMetaResult_T* pMetaResult) = 0;

    /**
     * @brief update P2 infot to resultpool
     * @param [in]  rP2Info is P2 information.
     */
    virtual MVOID                   updateP2Info(P2Info_T& rP2Info) = 0;

    /**
     * @brief get P2 infot to resultpool
     * @param [out]  rP2Info is P2 information.
     */
    virtual MVOID                   getP2Info(P2Info_T& rP2Info) = 0;

}; //IResultPool


/*********************************************************
*
*   Interface of ResultPoolImp
*
*********************************************************/
class ResultPoolImp : public IResultPool
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    ResultPoolImp(MINT32 i4SensorDevId);
    virtual ~ResultPoolImp();
    // copy constructor and copy assignment operator
    ResultPoolImp(const ResultPoolImp&);
    ResultPoolImp& operator=(const ResultPoolImp&);

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
    virtual const MVOID*            getResult(const MINT32& i4ReqMagic, const E_PARTIAL_RESULT_OF_MODULE_T& eModule, const char* pFunctionName);

    /**
     * @brief update partial of 3A result
     * @param [in]  strUserName is which module update; It refer to LOG_TAG.
     * @param [in]  i4ReqMagic is current request; It refer to magic number of set function
     * @param [in]  eModule is 3A enum; please refer to E_PARTIAL_RESULT_OF_MODULE_T
     * @param [in]  pData is partial of 3A result which module update.
     */
    virtual MINT32                  updateResult(std::string strUserName, const MINT32& i4ReqMagic, const E_PARTIAL_RESULT_OF_MODULE_T& eModule, const MVOID* pData);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces for 3A HAL.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief Push basic convert to meta enum to list and Call allocateResultPool()
     */
    virtual MVOID                   init();

    /**
     * @brief Clear list and Call freeResultPool()
     */
    virtual MVOID                   uninit();

    /**
     * @brief Update SensorIdx, SensorDev, SensorMode to local member.
     * @param [in]  rBasicCfg is sensor basic info.
     */
    virtual MVOID                   config(const BASIC_CFG_INFO_T& rBasicCfg);

    /**
     * @brief 3A update Capacity or not
     * @param [in]  u4Capacity is size of result buffer pool.
     * @param [in]  i4SubsampleCount is for slow motion.
     */
    virtual MINT32                  setCapacity(const MUINT32& u4Capacity, const MINT32& i4SubsampleCount);

    /**
     * @brief get config of result
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return CFG;
     */
    virtual REQUEST_INFO_T          getRequestInfo(const MINT32& i4ReqMagic);

    /**
     * @brief update config of result
     * @param [in]  i4ReqMagic is which you want request magic number, i4SttMagic is statistic magic number;
     * @param [out] return false or true;
     */
    virtual MBOOL                   setRequestInfo(const REQUEST_INFO_T& rRequestInfo);

    /**
     * @brief set Meta Static Info to resultPool
     * @param [in]  metaStaticInfo is meta info
     */
    virtual MVOID                   setMetaStaticInfo(const IMetadata& metaStaticInfo);

    /**
     * @brief get Meta Static Info to resultPool
     * @param [out]  metaStaticInfo is meta info
     */
    virtual IMetadata&              getMetaStaticInfo();

    /**
     * @brief update magic number history to resultPool
     * @param [in] i4ReqMagic is which you want request magic number.
     * @param [in] Size is buffer size.
     */
    virtual MVOID                   setHistory(const MINT32& i4ReqMagic, const MINT32& Size);

    /**
     * @brief get magic number history from resultPool
     * @param [out] pHistory is buffer.
     */
    virtual MINT32                  getHistory(MINT32* pHistory);

    /**
     * @brief Clear all result pool.
     */
    virtual MVOID                   clearAllResultPool();

    /**
     * @brief get all of 3A result
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return result which is all of 3A result. Or return NULL.
     */
    virtual AllResult_T*            getResultByReqNum(const MINT32& i4ReqMagic, const char* pFunctionName);

    /**
     * @brief get all of 3A result
     * @param [in]  i4SttMagic is which you want statistic magic number;
     * @param [out] return result which is all of 3A result. Or return NULL.
     */
    virtual AllResult_T*            getResultBySttNum(const MINT32& i4SttMagic, const char* pFunctionName);

    /**
     * @brief get all of 3A result
     * @param [in]  i4PresetKey is which you want presetkey number;
     * @param [out] return result which is all of 3A result. Or return NULL.
     */
    virtual AllResult_T*            getResultByPresetKey(MINT32 i4PreSetKey, const char* pFunctionName);

    /**
     * @brief return 3A result to result pool
     * @param [in]  all of 3A result
     */
    virtual MVOID                   returnResult(AllResult_T* pAllResult, const char* pFunctionName);

    /**
     * @brief get metadata result
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return metadata result. Or return default metadata result.
     */
    virtual AllMetaResult_T*        getMetadata(const MINT32& i4ReqMagic, const char* pFunctionName);

    /**
     * @brief return metadata to result pool
     * @param [in]  metadata
     */
    virtual VOID                    returnMetadata(AllMetaResult_T* pMetaResult, const char* pFunctionName);

    /**
     * @brief get validate result buffer
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return success or fail.
     */
    virtual MBOOL                   findValidateBuffer(MINT32 i4ReqMagic);

    /**
     * @brief If result is validate, then prepare result. For preview
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return false or true
     */
    virtual MINT32                  isValidateByReqNum(const MINT32& i4ReqMagic, const E_PARTIAL_RESULT_OF_MODULE_T& eModule);

    /**
     * @brief If result is validate, then prepare result For HQC
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return false or true
     */
    virtual MINT32                  isValidateBySttNum(const MINT32& i4SttMagic, const E_PARTIAL_RESULT_OF_MODULE_T& eModule);

    /**
     * @brief convert result to metadata for 2A
     * @param [in]  pAllResult is which you want to convert result;
     * @param [in]  pMetaResult is which you want to update metadata;
     */
    virtual MINT32                  convertToMetadata42A(AllResult_T* pAllResult, AllMetaResult_T* pMetaResult);

    /**
     * @brief convert result to metadata for AF
     * @param [in]  pAllResult is which you want to convert result;
     * @param [in]  pMetaResult is which you want to update metadata;
     */
    virtual MINT32                  convertToMetadata4AF(AllResult_T* pAllResult, AllMetaResult_T* pMetaResult);

    /**
     * @brief convert result to metadata flow
     * @param [in]  i4ReqMagic is which you want request magic number.
     * @param [in]  pAllResult is which you want to convert result;
     * @param [in]  pMetaResult is which you want to update metadata;
     */
    virtual MINT32                  convertToMetadataFlow(const MINT32& i4ReqMagic, MINT32 i4ConvertType, AllResult_T* pAllResult, AllMetaResult_T* pMetaResult);

    /**
     * @brief update P2 infot to resultpool
     * @param [in]  rP2Info is P2 information.
     */
    virtual MVOID                   updateP2Info(P2Info_T& rP2Info);

    /**
     * @brief get P2 infot to resultpool
     * @param [out]  rP2Info is P2 information.
     */
    virtual MVOID                   getP2Info(P2Info_T& rP2Info);

private:

    /**
     * @brief Clear furture update resultpool.
     * @param [in]  i4ReqMagic is now request magic number.
     */
    virtual MVOID                   clearOldestResultPool(const MINT32& i4ReqMagic, const MINT32& i4Index);

    /**
     * @brief Allocate Meta/3A result pool
     */
    virtual MVOID                   allocateResultPool();

    /**
     * @brief Free Meta/3A result pool
     */
    virtual MVOID                   freeResultPool();

    /**
     * @brief reset someone resultPool.
     * @param [in]  i4Indx is which you want i4Indx of request magic number;
     */
    virtual MVOID                   resetResultPool(const MINT32& i4Indx);

    /**
     * @brief magic mapping to index.
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return index of magic.
     */
    //virtual MINT32                  magic2Index(MINT32 i4ReqMagic);
    /**
     * @brief get new index by ring buffer.
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return index of magic.
     */
    virtual MINT32                  getValidateIndex(const MINT32& i4ReqMagic);
    /**
     * @brief get index by ring buffer.
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return index of magic.
     */
    virtual MINT32                  magic2IndexRingBuffer(const MINT32& i4ReqMagic);

    /**
     * @brief module update to meta.
     * @param [in]  vec3AROI is module ROI;
     * @param [in]  rMetaResult is MetaPool which ROI update.
     */
    MINT32                          convert3AROI(const std::vector<MINT32> &vec3AROI, AllMetaResult_T* rMetaResult, const mtk_camera_metadata_tag_t &tag) const;

    MINT32                          convertSubsampleResult4SMVR(const MINT32& i4ReqMagic, MINT32 i4ConvertType, AllResult_T* pAllResult, AllMetaResult_T* pMetaResult);

public:
    static MUINT32                  m_u4LogEn;

protected:
    AllResult_T*                    m_rResultPool;
    AllMetaResult_T*                m_rMetaResultPool;
    BASIC_CFG_INFO_T                m_rBasicCfg;
    MUINT32                         m_u4Capacity;
    IMetadata                       m_metaStaticInfo;
    IMPORTANT_INFO_T                m_r3AImportantInfo;
    MINT32                          m_i4SubsampleCount;
    std::mutex                      m_LockHistory;
    std::vector<MINT32>             m_vecHistory;
    MINT32                          m_i4CurIndex;
    std::mutex                      m_LockCurIndex;
    P2Info_T                        m_rP2Info;
    MINT32                          m_i4PreReqMagic;
};


} //namespace NS3Av3

#endif  //__IRESULT_POOL_H__
