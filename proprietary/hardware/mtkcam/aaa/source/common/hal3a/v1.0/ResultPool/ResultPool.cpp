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
* @file ResultPool.cpp
* @brief Implementation of Result Pool Manager
*/

#define LOG_TAG "ResultPool"

#include <mtkcam/utils/std/ULog.h>
//#include <cutils/properties.h>
//#include <private/aaa_hal_private.h>
//#include "aaa_utils.h"
//#include <debug_exif/dbg_id_param.h>

#include "IResultPool.h"
#include <ThreadResultPool.h>

#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/hw/HwTransform.h>

#include <private/aaa_hal_private.h>
#include <private/aaa_utils.h>

#include <mtkcam/utils/exif/IBaseCamExif.h>

//Module
#include <mtkcam/aaa/IDngInfo.h>
#if CAM3_LSC_FEATURE_EN
#include <lsc/ILscMgr.h>
#endif

//Systrace
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include <array>
#include <mtkcam/utils/std/ULog.h>

using namespace android;
using namespace NS3Av3;
using namespace NSCam;
using namespace NSCamHW;

/*******************************************************************************
*
*   utilities
*
********************************************************************************/
#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_3A_RESULTPOOL(String) \
          do { \
              aee_system_exception( \
                  "ResultPoolImp", \
                  NULL, \
                  DB_OPT_DEFAULT, \
                  String); \
          } while(0)
#else
#define AEE_ASSERT_3A_RESULTPOOL(String)
#endif
#define GET_PROP(prop, dft, val)\
{\
   val = property_get_int32(prop,dft);\
}

#define MY_LOGD(fmt, arg...) \
    do { \
        CAM_ULOGD(Utils::ULog::MOD_3A_FRAMEWORK_RESULT, fmt, ##arg); \
    }while(0)

#define MY_LOGD_IF(cond, ...) \
    do { \
        if ( (cond) ){ CAM_ULOGD(Utils::ULog::MOD_3A_FRAMEWORK_RESULT, __VA_ARGS__); } \
    }while(0)

#define MY_LOGW(fmt, arg...) \
    do { \
        CAM_ULOGW(Utils::ULog::MOD_3A_FRAMEWORK_RESULT, fmt, ##arg); \
    }while(0)

#define MY_LOGE(fmt, arg...) \
    do { \
        CAM_ULOGE(Utils::ULog::MOD_3A_FRAMEWORK_RESULT, fmt, ##arg); \
    }while(0)

#define MY_ASSERT(cond, ...) \
    CAM_ULOG_ASSERT(Utils::ULog::MOD_3A_FRAMEWORK_RESULT, cond, "[%s] %s, check fail !", __FUNCTION__, #cond);

/*******************************************************************************
*
*   LOG define
*
********************************************************************************/
#define HAL3RESULTPOOL_LOG_SET_0    (1<<0)  // Metadata
#define HAL3RESULTPOOL_LOG_SET_1    (1<<1)  // Result

#define HAL3RESULTPOOL_LOG_GET_0    (1<<2)  // Metadata
#define HAL3RESULTPOOL_LOG_GET_1    (1<<3)  // Result

#define HAL3RESULTPOOL_LOG_CVT_0    (1<<4)  // Convert

#define HAL3RESULTPOOL_LOG_AREA     (1<<5)  // ROI, Area
#define HAL3RESULTPOOL_LOG_PF       (1<<6)
#define HAL3RESULTPOOL_LOG_OTHER    (1<<7)  // reset, allocate, magic2index
#define HAL3RESULTPOOL_LOG_AAOINFO  (1<<8)
#define HAL3RESULTPOOL_LOG_LASTINFO (1<<9)


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
public:
    ResultPoolImp(MINT32 i4SensorIdx);
    virtual ~ResultPoolImp();

protected:
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
    virtual const MVOID*            getResult(MINT32 i4ReqMagic, E_PARTIAL_RESULT_OF_MODULE_T eModule);

    /**
     * @brief update partial of 3A result
     * @param [in]  strUserName is which module update; It refer to LOG_TAG.
     * @param [in]  i4ReqMagic is current request; It refer to magic number of set function
     * @param [in]  eModule is 3A enum; please refer to E_PARTIAL_RESULT_OF_MODULE_T
     * @param [in]  pData is partial of 3A result which module update.
     */
    virtual MINT32                  updateResult(std::string strUserName, MINT32 i4ReqMagic, E_PARTIAL_RESULT_OF_MODULE_T eModule, const MVOID* pData);

    /**
     * @brief get PSO info of cameInfo and LSC.
     * @param [in]  u4GridPointNum definition by Ae_mgr.
     * @param [in]  i4SttMagic is statistic magic number;
     * @param [out] rPSOInfo has PSO info.
     */
#if (!CAM3_3A_ISP_30_EN)
    virtual MBOOL                   getAAOInfo(MUINT32 u4GridPointNum, MINT32 i4SttMagic, AAO_PROC_INFO_T& rPSOInfo);
    virtual MBOOL                   getAAOInfoTest(MUINT32 u4GridPointNum, MINT32 i4SttMagic, AAO_PROC_INFO_T& rPSOInfo);
#endif
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
    virtual MINT32                  updateCapacity(MUINT32 u4Capacity, MINT32 i4SubsampleCount);

    /**
     * @brief get config of result
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return reference CFG;
     */
    virtual RESULT_CFG_T*           getResultCFG(MINT32 i4ReqMagic);

    /**
     * @brief update config of result
     * @param [in]  i4ReqMagic is which you want request magic number, i4SttMagic is statistic magic number;
     * @param [out] return false or true;
     */
    virtual MBOOL                   updateResultCFG(MINT32 i4ReqMagic, MINT32 i4SttMagic);

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
    virtual MVOID                   updateHistory(MINT32 i4ReqMagic, MINT32 Size);

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
    virtual AllResult_T*            getAllResult(MINT32 i4ReqMagic);

    /**
     * @brief get all of 3A result
     * @param [in]  i4SttMagic is which you want statistic magic number;
     * @param [out] return result which is all of 3A result. Or return NULL.
     */
    virtual AllResult_T*            getAllResultCur(MINT32 i4SttMagic);

    /**
     * @brief get all of 3A result. Avoid getCur function getting fail.
     *  Use i4ReqMagic 0 to get.
     */
    virtual AllResult_T*            getAllResultLastCur(MINT32 i4ReqMagic);

    /**
     * @brief update all of 3A result
     * @param [in]  strUserName is which module update; It refer to LOG_TAG.
     * @param [in]  i4ReqMagic is which you want to update request magic number;
     * @param [in]  pData is all of 3A result
     */
    virtual MINT32                  updateAllResult(std::string strUserName, MINT32 i4ReqMagic, const AllResult_T* pData);

    /**
     * @brief get metadata result
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return metadata result. Or return default metadata result.
     */
    virtual AllMetaResult_T*        getMetadata(MINT32 i4ReqMagic);

    /**
     * @brief get validate metadata result buffer
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return metadata result buffer.
     */
    virtual MetaSet_T*              getValidateMetadata(MINT32 i4ReqMagic);

    /**
     * @brief If result is validate, then prepare result. For preview
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return false or true
     */
    virtual MINT32                  isValidate(MINT32 i4ReqMagic);

    /**
     * @brief If result is validate, then prepare result For HQC
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return false or true
     */
    virtual MINT32                  isValidateCur(MINT32 i4SttMagic);

    /**
     * @brief update module state of early callBack.
     * @param [in]  i4ReqMagic is which you want request magic number.
     * @param [in]  rEarlyCB is which you want to update struct.
     * @param [out] return false or true;
     */
    virtual MBOOL                   updateEarlyCB(MINT32 i4ReqMagic, EARLY_CALL_BACK &rEarlyCB);

    /**
     * @brief update module state of early callBack.
     * @param [in]  i4ReqMagic is which you want request magic number.
     * @param [out] return reference EARLY_CALL_BACK;
     */
    virtual EARLY_CALL_BACK*        getEarlyCB(MINT32 i4ReqMagic);

    /**
     * @brief convert result to metadata for 2A
     * @param [in]  pAllResult is which you want to convert result;
     * @param [in]  pMetaResult is which you want to update metadata;
     */
    virtual MINT32                  convertToMetadata(AllResult_T* pAllResult, AllMetaResult_T* pMetaResult);

    /**
     * @brief convert result to metadata for AF
     * @param [in]  pAllResult is which you want to convert result;
     * @param [in]  pMetaResult is which you want to update metadata;
     */
    virtual MINT32                  convertToMetadata4AF(AllResult_T* pAllResult, AllMetaResult_T* pMetaResult);

    /**
     * @brief post to threadResultPool
     * @param [in] pCmd is resultPool command.
     */
    virtual MVOID                   postCmdToThread(ResultPoolCmd4Convert_T* pCmd);

    /**
     * @brief update LastInfo to result Pool
     * @param [in] LastInfo is camerainfo.
     */
    virtual MVOID                   updateLastInfo(LastInfo_T& vLastInfo);

    /**
     * @brief get LastInfo to result Pool
     * @param [out] return last info.
     */
    virtual LastInfo_T&             getLastInfo();
    /**
     * @brief get mutex for lock LastInfo buffer from result Pool
     * @param [out] return true.
     */
    virtual MBOOL                   lockLastInfo();

    /**
     * @brief get mutex for unlock LastInfo buffer from result Pool
     * @param [out] return true.
     */
    virtual MBOOL                   unlockLastInfo();

private:

    /**
     * @brief Clear furture update resultpool.
     * @param [in]  i4ReqMagic is now request magic number.
     */
    virtual MVOID                   clearOldestResultPool(MINT32 i4ReqMagic, MINT32 i4Index);

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
    virtual MVOID                   resetResultPool(MINT32 i4Indx);

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
    virtual MINT32                  getValidateIndex(MINT32 i4ReqMagic);
    /**
     * @brief get index by ring buffer.
     * @param [in]  i4ReqMagic is which you want request magic number;
     * @param [out] return index of magic.
     */
    virtual MINT32                  magic2IndexRingBuffer(MINT32 i4ReqMagic);

    /**
     * @brief module update to meta.
     * @param [in]  vec3AROI is module ROI;
     * @param [in]  rMetaResult is MetaPool which ROI update.
     */
    MINT32                          convert3AROI(const android::Vector<MINT32> &vec3AROI, AllMetaResult_T* rMetaResult, const mtk_camera_metadata_tag_t &tag) const;

public:
    static MUINT32                  m_u4LogEn;

protected:
    ThreadResultPool*               m_pThreadResultPool;
    AllResult_T*                    m_rResultPool;
    AllMetaResult_T*                m_rMetaResultPool;
    BASIC_CFG_INFO_T                m_rBasicCfg;
    MUINT32                         m_u4Capacity;
    AllMetaResult_T                 m_defaultMeta;
    IMetadata                       m_metaStaticInfo;
    IMPORTANT_INFO_T                m_r3AImportantInfo;
    MINT32                          m_i4SubsampleCount;
    Mutex                           m_LockHistory;
    List<MINT32>                    m_i4ValidateEnumList4Meta;
    android::Vector<MINT32>         m_vecHistory;
    MUINT16**                       m_pLSCGain;
    Mutex                           m_LockLastInfo;
    LastInfo_T                      m_vLastInfo;
    MINT32                          m_i4CurIndex;
    Mutex                           m_LockCurIndex;
};

#define MY_INST NS3Av3::INST_T<ResultPoolImp>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;
MUINT32 ResultPoolImp::m_u4LogEn = 0;

/*******************************************************************************
* Result Pool Implementation
********************************************************************************/
inline static
CameraArea_T _transformArea2Active(const MUINT32 i4SensorIdx, const MINT32 i4SensorMode, const CameraArea_T& rArea)
{
    if (rArea.i4Left == 0 && rArea.i4Top == 0 && rArea.i4Right == 0 && rArea.i4Bottom == 0)
    {
        return rArea;
    }
    MBOOL fgLogArea = (ResultPoolImp::m_u4LogEn & HAL3RESULTPOOL_LOG_AREA) ? MTRUE: MFALSE;
    CameraArea_T rOut;

    HwTransHelper helper(i4SensorIdx);
    HwMatrix mat;
    if(!helper.getMatrixToActive(i4SensorMode, mat))
        CAM_LOGE("Get hw matrix failed");
    if(fgLogArea)
        mat.dump(__FUNCTION__);

    MSize size(rArea.i4Right- rArea.i4Left,rArea.i4Bottom- rArea.i4Top);
    MPoint point(rArea.i4Left, rArea.i4Top);
    MRect input(point,size);
    MRect output;
    mat.transform(input, output);

    rOut.i4Left   = output.p.x;
    rOut.i4Right  = output.p.x + output.s.w;
    rOut.i4Top    = output.p.y;
    rOut.i4Bottom = output.p.y + output.s.h;
    rOut.i4Weight = rArea.i4Weight;

    CAM_LOGD_IF(fgLogArea, "[%s] in(%d,%d,%d,%d), out(%d,%d,%d,%d)", __FUNCTION__,
              input.p.x, input.p.y, input.s.w, input.s.h,
              output.p.x, output.p.y, output.s.w, output.s.h);
    CAM_LOGD_IF(fgLogArea, "[%s] rArea(%d,%d,%d,%d), rOut(%d,%d,%d,%d)", __FUNCTION__,
              rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom,
              rOut.i4Left, rOut.i4Top, rOut.i4Right, rOut.i4Bottom);
    return rOut;
}

MINT32
ResultPoolImp::
convert3AROI(const android::Vector<MINT32> &vec3AROI, AllMetaResult_T* pMetaResult, const mtk_camera_metadata_tag_t &tag) const
{
    MBOOL fgLogArea = (ResultPoolImp::m_u4LogEn & HAL3RESULTPOOL_LOG_AREA) ? MTRUE: MFALSE;

    // order is Type,Number of ROI,left,top,right,bottom,Result, left,top,right,bottom,Result...
    CAM_LOGD_IF(fgLogArea, "[%s] size(%d) pMetaResult:%p", __FUNCTION__, vec3AROI.size(), pMetaResult);
    if( vec3AROI.size()!=0)
    {
        IMetadata::IEntry entryNew(tag);
        const MINT32 *ptrROIInfo = vec3AROI.array();
        CameraArea_T rROI;

        MINT32 i4Type   = *ptrROIInfo++;
        MINT32 i4ROINum = *ptrROIInfo++;
        MINT32 i4Result = 0;

        entryNew.push_back(i4Type,      Type2Type<MINT32>());
        entryNew.push_back(i4ROINum,    Type2Type<MINT32>());

        CAM_LOGD_IF(fgLogArea,"[%s] ROINum(%d)", __FUNCTION__, i4ROINum);

        if(i4ROINum==0)
        {
            entryNew.push_back(0,   Type2Type<MINT32>());
            entryNew.push_back(0,   Type2Type<MINT32>());
            entryNew.push_back(0,   Type2Type<MINT32>());
            entryNew.push_back(0,   Type2Type<MINT32>());
            entryNew.push_back(0,   Type2Type<MINT32>());
        }

        for(MINT32 i = 0; i < i4ROINum; i++)
        {
            rROI.i4Left   = *ptrROIInfo++;
            rROI.i4Top    = *ptrROIInfo++;
            rROI.i4Right  = *ptrROIInfo++;
            rROI.i4Bottom = *ptrROIInfo++;
            i4Result      = *ptrROIInfo++;
            rROI.i4Weight = 0;

            CAM_LOGD_IF(fgLogArea,"[%s] ROINum(%d), ROIIndx(%d), Left(%d), Top(%d), Right(%d), Bottom(%d)"
                , __FUNCTION__, i4ROINum, i, rROI.i4Left, rROI.i4Top, rROI.i4Right, rROI.i4Bottom);

            rROI = _transformArea2Active(m_rBasicCfg.i4SensorIdx, m_rBasicCfg.i4SensorMode, rROI);

            CAM_LOGD_IF(fgLogArea,"[%s] Active Left(%d), Top(%d), Right(%d), Bottom(%d)"
                , __FUNCTION__, rROI.i4Left, rROI.i4Top, rROI.i4Right, rROI.i4Bottom);

            entryNew.push_back(rROI.i4Left,   Type2Type<MINT32>());
            entryNew.push_back(rROI.i4Top,    Type2Type<MINT32>());
            entryNew.push_back(rROI.i4Right,  Type2Type<MINT32>());
            entryNew.push_back(rROI.i4Bottom, Type2Type<MINT32>());
            entryNew.push_back(i4Result,      Type2Type<MINT32>());
        }

        pMetaResult->rMetaResult.appMeta.update(tag, entryNew);

        return MTRUE;
    }
    else
        return MFALSE;
}

MINT32
ResultPoolImp::
convertToMetadata4AF(AllResult_T* pAllResult, AllMetaResult_T* pMetaResult)
{
    AAA_TRACE_HAL(convertToMetadata4AF);

    AAA_TRACE_HAL(assigneValue4AF);
    MBOOL fgLogCVT = (m_u4LogEn & HAL3RESULTPOOL_LOG_CVT_0) ? MTRUE: MFALSE;
    MBOOL fgLogArea = (m_u4LogEn & HAL3RESULTPOOL_LOG_AREA) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLogCVT, "[%s]+ AllResult/MetaResult Addr:%p/%p", __FUNCTION__, pAllResult, pMetaResult);
    AAA_TRACE_END_HAL;

    Mutex::Autolock lock(pMetaResult->LockMetaResult);

    AAA_TRACE_HAL(ModuleResultAddr4AF);
    const HALResultToMeta_T*   pHALResult = ( (HALResultToMeta_T*)(pAllResult->ModuleResultAddr[E_HAL_RESULTTOMETA]->read()) );
    const AFResultToMeta_T*    pAFResult = ( (AFResultToMeta_T*)(pAllResult->ModuleResultAddr[E_AF_RESULTTOMETA]->read()) );
    CAM_LOGD_IF(fgLogCVT, "[%s] Req (#%d), pHALResult(%p), pAFResult(%p)", __FUNCTION__, pMetaResult->rMetaResult.MagicNum, pHALResult, pAFResult);
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(queryAFInfoToCompare);
    // query appMeta info to compare
    MUINT8 u1AfTrig = 0;
    QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_CONTROL_AF_TRIGGER, u1AfTrig);

    if(m_r3AImportantInfo.u1AfState != pAFResult->u1AfState || m_r3AImportantInfo.u1AfTrig != u1AfTrig)
    {
        CAM_LOGD("[%s] Req (#%d) AfState(%d->%d) AfTrig(%d->%d)",
            __FUNCTION__, pMetaResult->rMetaResult.MagicNum, m_r3AImportantInfo.u1AfState, pAFResult->u1AfState, m_r3AImportantInfo.u1AfTrig, u1AfTrig);
    }
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(convertAFInfoToMeta);
    // convert result into metadata
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_CONTROL_AF_STATE, pAFResult->u1AfState);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_LENS_STATE, pAFResult->u1LensState);
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(convertLensToMeta);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_LENS_FOCUS_DISTANCE, pAFResult->fLensFocusDistance);
    UPDATE_ENTRY_ARRAY(pMetaResult->rMetaResult.appMeta, MTK_LENS_FOCUS_RANGE, pAFResult->fLensFocusRange, 2);
    CAM_LOGD_IF(fgLogCVT, "[%s] FocusDist(%f), FocusRange(%f, %f)", __FUNCTION__, pAFResult->fLensFocusDistance, pAFResult->fLensFocusRange[0], pAFResult->fLensFocusRange[1]);
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(convertAFExifToMeta);
    // Exif
    {
        // protect vector before use vector
        Mutex::Autolock Vec_lock(pAllResult->LockVecResult);
        MINT32 i4VecExifSize = pAllResult->vecExifInfo.size();
        if (i4VecExifSize)
        {
            IMetadata metaExif;
            QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_3A_EXIF_METADATA, metaExif);
            //UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_FNUMBER,              fFNum*10/*rExifInfo.u4FNumber*/);
            //UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_FOCAL_LENGTH,         fFocusLength*1000/*rExifInfo.u4FocalLength*/);

            // debug info
            if (pAllResult->vecDbg3AInfo.size())
            {
                IMetadata::Memory dbg3A;
                //AAA_DEBUG_INFO2_T& rDbgIspInfo = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(pAllResult->vecDbgIspInfo.editArray());
                //IAfMgr::getInstance().getDebugInfo(m_rBasicCfg.u4SensorDev, rDbgIspInfo.rAFDebugInfo);
                dbg3A.appendVector(pAllResult->vecDbg3AInfo);
                UPDATE_ENTRY_SINGLE(metaExif, MTK_3A_EXIF_DBGINFO_AAA_DATA, dbg3A);
            }
            UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_3A_EXIF_METADATA, metaExif);
        }
        else
        {
            CAM_LOGD_IF(fgLogCVT, "[%s] vecExifInfo size(%d)", __FUNCTION__, i4VecExifSize);
        }
    }
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(printAFRegion);
    // log of control
    MINT32 rRgn[5] = {};
    if (GET_ENTRY_ARRAY(pMetaResult->rMetaResult.appMeta, MTK_CONTROL_AF_REGIONS, rRgn, 5))
    {
        CAM_LOGD_IF(fgLogCVT, "[%s] MTK_CONTROL_AF_REGIONS(%d,%d,%d,%d,%d)", __FUNCTION__,
            rRgn[0], rRgn[1], rRgn[2], rRgn[3], rRgn[4]);
    }
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(convertFocusAreaToMeta);
    //Focus area result.
    if( pAllResult->vecFocusAreaPos.size()!=0)
    {
        MINT32  szW = pAFResult->i4FocusAreaSz[0];
        MINT32  szH = pAFResult->i4FocusAreaSz[1];
        const MINT32 *ptrPos = pAllResult->vecFocusAreaPos.array();
        MINT32  cvtX = 0, cvtY = 0, cvtW = 0, cvtH = 0;
        android::Vector<MINT32> vecCvtPos = pAllResult->vecFocusAreaPos;
        MINT32 *ptrCvtPos = vecCvtPos.editArray();
        szW = szW/2;
        szH = szH/2;
        for( MUINT32 i=0; i<pAllResult->vecFocusAreaPos.size(); i+=2)
        {
            CameraArea_T pos;
            MINT32 X = *ptrPos++;
            MINT32 Y = *ptrPos++;

            pos.i4Left   = X - szW;
            pos.i4Top    = Y - szH;
            pos.i4Right  = X + szW;
            pos.i4Bottom = Y + szH;
            pos.i4Weight = 0;

            pos = _transformArea2Active(m_rBasicCfg.i4SensorIdx, m_rBasicCfg.i4SensorMode, pos);
            cvtX = (pos.i4Left + pos.i4Right )/2;
            cvtY = (pos.i4Top  + pos.i4Bottom)/2;
            *ptrCvtPos++ = cvtX;
            *ptrCvtPos++ = cvtY;

            cvtW = pos.i4Right  -  pos.i4Left;
            cvtH = pos.i4Bottom -  pos.i4Top;
        }
        if(fgLogCVT)
        {
            for( MUINT32 i=0; i<vecCvtPos.size(); i++)
            {
                CAM_LOGD_IF(fgLogCVT, "AF_REGIONS Pos %d(%d)", pAllResult->vecFocusAreaPos[i], vecCvtPos[i]);
            }

            for( MUINT32 i=0; i<pAllResult->vecFocusAreaRes.size(); i++)
            {
                CAM_LOGD_IF(fgLogCVT, "AF_REGIONS  Res (%d)", pAllResult->vecFocusAreaRes[i]);
            }
            CAM_LOGD_IF(fgLogCVT, "AF_REGIONS  W %d(%d), H %d(%d)", pAFResult->i4FocusAreaSz[0], cvtW, pAFResult->i4FocusAreaSz[1], cvtH);
        }
        MSize areaCvtSZ(cvtW, cvtH);
        UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_FOCUS_AREA_SIZE, areaCvtSZ);
        UPDATE_ENTRY_ARRAY(pMetaResult->rMetaResult.halMeta, MTK_FOCUS_AREA_POSITION, vecCvtPos.array(), vecCvtPos.size());
        UPDATE_ENTRY_ARRAY(pMetaResult->rMetaResult.halMeta, MTK_FOCUS_AREA_RESULT, pAllResult->vecFocusAreaRes.array(), pAllResult->vecFocusAreaRes.size());
    }
    AAA_TRACE_END_HAL;

    {
        Mutex::Autolock Vec_lock(pAllResult->LockVecResult);
        AAA_TRACE_HAL(convertAFROI);
        CAM_LOGD_IF(fgLogArea, "[%s] MTK_3A_FEATURE_AF_ROI", __FUNCTION__);
        convert3AROI(pAllResult->vecAFROI, pMetaResult, MTK_3A_FEATURE_AF_ROI);
        AAA_TRACE_END_HAL;
    }

    AAA_TRACE_HAL(updateImportantInfo4AF);
    // update Important Info as pre-info

    m_r3AImportantInfo.u1AfState = pAFResult->u1AfState;
    QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_CONTROL_AF_TRIGGER, u1AfTrig);
    m_r3AImportantInfo.u1AfTrig = u1AfTrig;
    AAA_TRACE_END_HAL;

    pMetaResult->i4IsConverted |= E_AF_CONVERT;

    CAM_LOGD_IF(fgLogCVT, "[%s]- IsConverted(%d)", __FUNCTION__, pMetaResult->i4IsConverted);
    AAA_TRACE_END_HAL;
    return MFALSE;

}

MINT32
ResultPoolImp::
convertToMetadata(AllResult_T* pAllResult, AllMetaResult_T* pMetaResult)
{
    AAA_TRACE_HAL(assigneValue42A);
    MBOOL fgLogCVT = (m_u4LogEn & HAL3RESULTPOOL_LOG_CVT_0) ? MTRUE: MFALSE;
    MBOOL fgLogArea = (m_u4LogEn & HAL3RESULTPOOL_LOG_AREA) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLogCVT, "[%s]+ AllResult/MetaResult Addr:%p/%p", __FUNCTION__, pAllResult, pMetaResult);
    AAA_TRACE_END_HAL;

    Mutex::Autolock lock(pMetaResult->LockMetaResult);

    AAA_TRACE_HAL(ModuleResultAddr42A);
    const HALResultToMeta_T*   pHALResult = ( (HALResultToMeta_T*)(pAllResult->ModuleResultAddr[E_HAL_RESULTTOMETA]->read()) );
    const AEResultToMeta_T*    pAEResult = ( (AEResultToMeta_T*)(pAllResult->ModuleResultAddr[E_AE_RESULTTOMETA]->read()) );
    const AWBResultToMeta_T*   pAWBResult = ( (AWBResultToMeta_T*)(pAllResult->ModuleResultAddr[E_AWB_RESULTTOMETA]->read()) );
    const ISPResultToMeta_T*   pISPResult = ( (ISPResultToMeta_T*)(pAllResult->ModuleResultAddr[E_ISP_RESULTTOMETA]->read()) );
    const LSCResultToMeta_T*   pLSCResult = ( (LSCResultToMeta_T*)(pAllResult->ModuleResultAddr[E_LSC_RESULTTOMETA]->read()) );
    const FLASHResultToMeta_T* pFLASHResult = ( (FLASHResultToMeta_T*)(pAllResult->ModuleResultAddr[E_FLASH_RESULTTOMETA]->read()) );
    const FLKResultToMeta_T*   pFLKResult = ( (FLKResultToMeta_T*)(pAllResult->ModuleResultAddr[E_FLK_RESULTTOMETA]->read()) );
    const Sync3AResultToMeta_T* pSync3AResult = ( (Sync3AResultToMeta_T*)(pAllResult->ModuleResultAddr[E_SYNC3A_RESULTTOMETA]->read()) );
    CAM_LOGD_IF(fgLogCVT, "[%s] Req (#%d) CurrTblIndex:%d", __FUNCTION__, pHALResult->i4FrmId, pLSCResult->i4CurrTblIndex);
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(convertAFLensToMeta);
    // 3A Thread to convert lens StaticInfo
    MFLOAT fFNum = 0.0, fFocusLength = 0.0;
    if (QUERY_ENTRY_SINGLE(m_metaStaticInfo, MTK_LENS_INFO_AVAILABLE_APERTURES, fFNum))
    {
        UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_LENS_APERTURE, fFNum);
    }
    else
        CAM_LOGE("[%s] Req (#%d) Query fFNum(%.5f) fail", __FUNCTION__, pHALResult->i4FrmId, fFNum);

    if (QUERY_ENTRY_SINGLE(m_metaStaticInfo, MTK_LENS_INFO_AVAILABLE_FOCAL_LENGTHS, fFocusLength))
    {
        UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_LENS_FOCAL_LENGTH, fFocusLength);
    }
    else
        CAM_LOGE("[%s] Req (#%d) Query fFocusLength(%.5f) fail", __FUNCTION__, pHALResult->i4FrmId, fFocusLength);
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(query2AInfoToCompare);
    // query appMeta info to compare
    MUINT8 u1CapIntent;
    MUINT8 u1AePrecapTrig;
    MUINT8 u1AWBMode = MTK_CONTROL_AWB_MODE_AUTO;
    QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_CONTROL_CAPTURE_INTENT, u1CapIntent);
    QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_CONTROL_AE_PRECAPTURE_TRIGGER, u1AePrecapTrig);
    QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_CONTROL_AWB_MODE, u1AWBMode);

    if(m_r3AImportantInfo.u1AeState != pAEResult->u1AeState || m_r3AImportantInfo.u1AwbState != pAWBResult->u1AwbState ||
        m_r3AImportantInfo.u1FlashState != pFLASHResult->u1FlashState || m_r3AImportantInfo.i8SensorExposureTime != pAEResult->i8SensorExposureTime ||
        m_r3AImportantInfo.i8SensorFrameDuration != pAEResult->i8SensorFrameDuration || m_r3AImportantInfo.i4SensorSensitivity != pAEResult->i4SensorSensitivity ||
        m_r3AImportantInfo.u1CapIntent != u1CapIntent || m_r3AImportantInfo.u1AePrecapTrig != u1AePrecapTrig || m_r3AImportantInfo.u1AWBMode != u1AWBMode ||
        m_r3AImportantInfo.u1ColorCorrectMode != pISPResult->rCamInfo.eColorCorrectionMode)
    {
        CAM_LOGD("[%s] Req(#%d) AeState(%d->%d) AwbState(%d->%d) FlashState(%d->%d) ExpTime(%lld->%lld) FrmDuration(%lld->%lld) ISO(%d->%d) CapIntent(%d->%d) AePrecapTrig(%d->%d) (fFNum, fFocusLength)(%.5f, %.5f) AWBMode(%d->%d) CCMMode(%d->%d)",
            __FUNCTION__, pHALResult->i4FrmId, m_r3AImportantInfo.u1AeState, pAEResult->u1AeState, m_r3AImportantInfo.u1AwbState, pAWBResult->u1AwbState,
            m_r3AImportantInfo.u1FlashState, pFLASHResult->u1FlashState, m_r3AImportantInfo.i8SensorExposureTime, pAEResult->i8SensorExposureTime,
            m_r3AImportantInfo.i8SensorFrameDuration, pAEResult->i8SensorFrameDuration, m_r3AImportantInfo.i4SensorSensitivity, pAEResult->i4SensorSensitivity,
            m_r3AImportantInfo.u1CapIntent, u1CapIntent, m_r3AImportantInfo.u1AePrecapTrig, u1AePrecapTrig, fFNum, fFocusLength, m_r3AImportantInfo.u1AWBMode, u1AWBMode,
            m_r3AImportantInfo.u1ColorCorrectMode, pISPResult->rCamInfo.eColorCorrectionMode);
    }
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(convert2AInfoToMeta);
    // convert result into metadata
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_CONTROL_SCENE_MODE, pHALResult->u1SceneMode);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_CONTROL_AWB_STATE, pAWBResult->u1AwbState);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_CONTROL_AE_STATE, pAEResult->u1AeState);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_FLASH_STATE, pFLASHResult->u1FlashState);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_STATISTICS_SCENE_FLICKER, pFLKResult->u1SceneFlk);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_HDR_FEATURE_HDR_DETECTION_RESULT, pAEResult->i4AutoHdrResult);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_3A_AE_BV_TRIGGER, pAEResult->fgAeBvTrigger);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_3A_FEATURE_AE_SENSOR_GAIN_VALUE, pAEResult->i4SensorGain);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_3A_FEATURE_AE_ISP_GAIN_VALUE, pAEResult->i4IspGain);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_3A_FEATURE_AE_LUX_INDEX_VALUE, pAEResult->i4LuxIndex);
    if(pFLASHResult->bCustEnableFlash)
        UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_3A_SKIP_PRECAPTURE, pFLASHResult->bCustEnableFlash);

    // flash calibration result
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_FLASH_FEATURE_CALIBRATION_RESULT, pFLASHResult->i4FlashCalResult);

    MUINT8 u1SubFlashCustomization;
    if (QUERY_ENTRY_SINGLE(m_metaStaticInfo, MTK_FLASH_FEATURE_CUSTOMIZATION_AVAILABLE, u1SubFlashCustomization))
        if(u1SubFlashCustomization)
            UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_FLASH_FEATURE_CUSTOMIZED_RESULT, pFLASHResult->u1SubFlashState);

    AAA_TRACE_END_HAL;
    MUINT8 aeMeteringMode[3] = {0, 1, 2};
    UPDATE_ENTRY_ARRAY(pMetaResult->rMetaResult.appMeta, MTK_3A_FEATURE_AE_AVAILABLE_METERING, aeMeteringMode, 3);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_3A_FEATURE_AE_AVERAGE_BRIGHTNESS, pAEResult->i4AvgY);
    AAA_TRACE_HAL(convertSensorInfoToMeta);
    // sensor
    IDngInfo* pDngInfo = nullptr;
    IMetadata rMetaDngDynNoiseProfile = {};
    pDngInfo = MAKE_DngInfo(LOG_TAG, m_rBasicCfg.i4SensorIdx);
    MY_ASSERT(pDngInfo != nullptr);

    if (pDngInfo)
    {
        MUINT8 color_filter_arrangement;
        QUERY_ENTRY_SINGLE(m_metaStaticInfo, MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT, color_filter_arrangement);
        if (color_filter_arrangement ==  MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT_MONO){ //MONO sensor
            IMetadata rMeta = pDngInfo->getDynamicNoiseProfile(pAEResult->i4SensorSensitivity);
            double noise_profile_mono_temp[8] = {0.0};
            GET_ENTRY_ARRAY(rMeta, MTK_SENSOR_NOISE_PROFILE, noise_profile_mono_temp, 8);
            double noise_profile_mono[2] = {noise_profile_mono_temp[0], noise_profile_mono_temp[1]};
            UPDATE_ENTRY_ARRAY(rMetaDngDynNoiseProfile, MTK_SENSOR_NOISE_PROFILE, noise_profile_mono, 2);
            NSCam::IMetadata::IEntry entry = rMetaDngDynNoiseProfile.entryFor(MTK_SENSOR_NOISE_PROFILE);
            CAM_LOGD("update MTK_SENSOR_NOISE_PROFILE with %d count", entry.count());
        } else {
            rMetaDngDynNoiseProfile = pDngInfo->getDynamicNoiseProfile(pAEResult->i4SensorSensitivity);
        }
     }

    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_SENSOR_EXPOSURE_TIME, pAEResult->i8SensorExposureTime);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_SENSOR_FRAME_DURATION, pAEResult->i8SensorFrameDuration);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_SENSOR_SENSITIVITY, pAEResult->i4SensorSensitivity);
    MUINT8 u1RollingShutterSkew = GET_ENTRY_SINGLE_IN_ARRAY(m_metaStaticInfo, MTK_REQUEST_AVAILABLE_RESULT_KEYS, (MINT32)MTK_SENSOR_ROLLING_SHUTTER_SKEW);
    CAM_LOGD_IF(fgLogCVT, "[%s] RSS(%lld) ExpTime(%lld) FrmDuration(%lld) ISO(%d)", __FUNCTION__,
                pAEResult->i8SensorRollingShutterSkew, pAEResult->i8SensorExposureTime, pAEResult->i8SensorFrameDuration, pAEResult->i4SensorSensitivity);

    MUINT32 u4RawFmtType = SENSOR_RAW_Bayer;
    //Before phone boot up (before opening camera), we can query IHalsensor for the sensor static info (EX: MONO or Bayer)
    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    if (pHalSensorList)
    {
        SensorStaticInfo sensorStaticInfo;
        pHalSensorList->querySensorStaticInfo(m_rBasicCfg.u4SensorDev, &sensorStaticInfo);
        u4RawFmtType = sensorStaticInfo.rawFmtType; // SENSOR_RAW_MONO or SENSOR_RAW_Bayer
    } else {
        MY_LOGE("MAKE_HalSensorList() == NULL");
    }

    if(u4RawFmtType != SENSOR_RAW_MONO)
    {
        UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_SENSOR_GREEN_SPLIT, (MFLOAT) 0.0f);
    }

    pMetaResult->rMetaResult.appMeta += rMetaDngDynNoiseProfile;
    AAA_TRACE_END_HAL;

    // Awb
    AAA_TRACE_HAL(convertAWBInfoToMeta);
    MRational NeutralColorPt[3];
    for (MINT32 k = 0; k < 3; k++)
    {
        NeutralColorPt[k].denominator = pAWBResult->i4AwbGain[k];
        NeutralColorPt[k].numerator = pAWBResult->i4AwbGainScaleUint;
    }
    if(u4RawFmtType != SENSOR_RAW_MONO)
    {
        UPDATE_ENTRY_ARRAY(pMetaResult->rMetaResult.appMeta, MTK_SENSOR_NEUTRAL_COLOR_POINT, NeutralColorPt, 3);
    }
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_COLOR_CORRECTION_ABERRATION_MODE, (MUINT8) MTK_COLOR_CORRECTION_ABERRATION_MODE_OFF);
    if(u4RawFmtType != SENSOR_RAW_MONO)
    {
        UPDATE_ENTRY_ARRAY(pMetaResult->rMetaResult.appMeta, MTK_COLOR_CORRECTION_GAINS, pAWBResult->fColorCorrectGain, 4);
    }
    UPDATE_ENTRY_ARRAY(pMetaResult->rMetaResult.appMeta, MTK_3A_FEATURE_AWB_AVAILABL_RANGE, pAWBResult->i4AwbAvailableRange, 2);
    CAM_LOGD_IF(fgLogCVT, "[%s] Awb Gain(%3.6f, %3.6f, %3.6f, %3.6f), Awb Available Range(%d, %d)", __FUNCTION__,
            pAWBResult->fColorCorrectGain[0], pAWBResult->fColorCorrectGain[1], pAWBResult->fColorCorrectGain[2], pAWBResult->fColorCorrectGain[3],
            pAWBResult->i4AwbAvailableRange[0], pAWBResult->i4AwbAvailableRange[1]);

    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_3A_MANUAL_AWB_COLORTEMPERATURE_MAX, pAWBResult->i4MWBColorTemperatureMax);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_3A_MANUAL_AWB_COLORTEMPERATURE_MIN, pAWBResult->i4MWBColorTemperatureMin);
    AAA_TRACE_END_HAL;

    if(u4RawFmtType != SENSOR_RAW_MONO)
    {
        AAA_TRACE_HAL(convertColCrrMToMeta);
        // color correction matrix
        if (pAllResult->vecColorCorrectMat.size())
        {
            const MFLOAT* pfMat = pAllResult->vecColorCorrectMat.array();
            IMetadata::IEntry entry(MTK_COLOR_CORRECTION_TRANSFORM);
            for (MINT32 k = 0; k < 9; k++)
            {
                MRational rMat;
                MFLOAT fVal = *pfMat++;
                rMat.numerator = fVal*512;
                rMat.denominator = 512;
                entry.push_back(rMat, Type2Type<MRational>());
                CAM_LOGD_IF(fgLogCVT, "[%s] Mat[%d] = (%3.6f, %d)", __FUNCTION__, k, fVal, rMat.numerator);
            }
            pMetaResult->rMetaResult.appMeta.update(MTK_COLOR_CORRECTION_TRANSFORM, entry);
        }
        AAA_TRACE_END_HAL;
    }
    if(u4RawFmtType != SENSOR_RAW_MONO)
    {
        UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_COLOR_CORRECTION_MODE, (MUINT8)(pISPResult->rCamInfo.eColorCorrectionMode));
    }

    AAA_TRACE_HAL(convertLSCInfoToMeta);
        IMetadata::Memory rLscData;
    // shading
    if(pLSCResult->i4CurrTblIndex != -1)
    {
        NSIspTuning::ILscMgr* pLsc = NSIspTuning::ILscMgr::getInstance(static_cast<MUINT32>(m_rBasicCfg.u4SensorDev));
        Vector<MUINT8> d;
        pLsc->getCurrTbl(pLSCResult->i4CurrTblIndex, d);
        rLscData.appendVector(d);

        MINT32 i4Size = rLscData.size();
        CAM_LOGD_IF(fgLogCVT, "[%s] CurrTblIndex:%d Size:%d", __FUNCTION__, pLSCResult->i4CurrTblIndex, i4Size);
        UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_LSC_TBL_DATA, rLscData);
    }
    if (pLSCResult->fgShadingMapOn)
    {
        CAM_LOGD("fgShadingMapOn prepare MTK_STATISTICS_LENS_SHADING_MAP");
        MUINT8 u1ShadingMode = 0;
        QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_SHADING_MODE, u1ShadingMode);
        if (pDngInfo)
        {
            pMetaResult->rMetaResult.appMeta += pDngInfo->getShadingMapFromMem(u1ShadingMode, rLscData);//ToDo other file
        }
    }
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(convertTuningInfoToMeta);
    // tuning
    UPDATE_MEMORY(pMetaResult->rMetaResult.halMeta, MTK_PROCESSOR_CAMINFO, pISPResult->rCamInfo);

    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_3A_FEATURE_ADRC_GAIN_VALUE, pISPResult->i4LceGain);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_3A_ISP_LCE_GAIN, pISPResult->i4LceGain);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_REAL_LV, pISPResult->rCamInfo.rAEInfo.i4RealLightValue_x10);

    MINT32 focusLength35mm = 0;
    MBOOL chk = QUERY_ENTRY_SINGLE(m_metaStaticInfo, MTK_3A_FEATURE_LENS_AVAILABLE_FOCAL_LENGTHS_35MM, focusLength35mm);
    CAM_LOGD_IF(fgLogCVT, "[%s] focusLength35mm(%d), chk(%d), i4LceGain(%d)", __FUNCTION__, focusLength35mm, chk, pISPResult->i4LceGain);

    if(pSync3AResult && pSync3AResult->rDualZoomInfo.i4AELv_x10 && pSync3AResult->rDualZoomInfo.i4AEIso){
        UPDATE_MEMORY(pMetaResult->rMetaResult.halMeta, MTK_STEREO_SYNC2A_STATUS, pSync3AResult->rDualZoomInfo);
        CAM_LOGD_IF(fgLogCVT, "[%s] AE(%d/%d/%d/%d) AF(%d) Sync2A(%d/%d)", __FUNCTION__,
            pSync3AResult->rDualZoomInfo.i4AELv_x10,
            pSync3AResult->rDualZoomInfo.i4AEIso,
            pSync3AResult->rDualZoomInfo.bIsAEBvTrigger,
            pSync3AResult->rDualZoomInfo.bIsAEAPLock,
            pSync3AResult->rDualZoomInfo.i4AFDAC,
            pSync3AResult->rDualZoomInfo.bSyncAFDone,
            pSync3AResult->rDualZoomInfo.bSync2ADone);
    }

    //UPDATE_ENTRY_ARRAY(rMetaResult.halMeta, MTK_PROCESSOR_CAMINFO, rResult.rCamInfo.data, sizeof(NSIspTuning::RAWIspCamInfo_U));
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(convert2AExifToMeta);
    // Exif
    {
        // protect vector before use vector
        Mutex::Autolock Vec_lock(pAllResult->LockVecResult);
        if (pAllResult->vecExifInfo.size())
        {
            IMetadata metaExif;
            QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_3A_EXIF_METADATA, metaExif);
            const EXIF_3A_INFO_T& rExifInfo = pAllResult->vecExifInfo[0];
            //UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_SCENE_MODE,           rExifInfo.u4SceneMode);
            UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_AWB_MODE,             rExifInfo.u4AWBMode);
            UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_LIGHT_SOURCE,         rExifInfo.u4LightSource);
            UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_EXP_PROGRAM,          rExifInfo.u4ExpProgram);
            UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_SCENE_CAP_TYPE,       rExifInfo.u4SceneCapType);
            UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_FLASH_LIGHT_TIME_US,  rExifInfo.u4FlashLightTimeus);
            UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_AE_METER_MODE,        rExifInfo.u4AEMeterMode);
            UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_AE_EXP_BIAS,          rExifInfo.i4AEExpBias);
            UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_CAP_EXPOSURE_TIME,    rExifInfo.u4CapExposureTime);
            UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_AE_ISO_SPEED,         rExifInfo.u4AEISOSpeed);
            //UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_REAL_ISO_VALUE,       rExifInfo.u4RealISOValue);

            // lens
            UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_FNUMBER,              fFNum*FNUMBER_PRECISION/*rExifInfo.u4FNumber*/);
            UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_FOCAL_LENGTH,         fFocusLength*1000/*rExifInfo.u4FocalLength*/);
            UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_FOCAL_LENGTH_35MM,    focusLength35mm);

            // debug info
            if (pAllResult->vecDbg3AInfo.size() && pAllResult->vecDbgShadTbl.size() && pAllResult->vecDbgIspInfo.size())
            {
                CAM_LOGD_IF(fgLogCVT, "[%s] DebugInfo #(%d)", __FUNCTION__, pHALResult->i4FrmId);
                UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_DBGINFO_AAA_KEY, AAA_DEBUG_KEYID);
                UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_DBGINFO_SDINFO_KEY, DEBUG_SHAD_TABLE_KEYID);
                UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_DBGINFO_ISP_KEY, ISP_DEBUG_KEYID);
                IMetadata::Memory dbg3A;
                //IMetadata::Memory dbgShd;
                IMetadata::Memory dbgIspP1;
                dbg3A.appendVector(pAllResult->vecDbg3AInfo);
                //dbgShd.appendVector(pAllResult->vecDbgShadTbl);
                dbgIspP1.appendVector(pAllResult->vecDbgIspInfo);
                UPDATE_ENTRY_SINGLE(metaExif, MTK_3A_EXIF_DBGINFO_AAA_DATA, dbg3A);
                //UPDATE_ENTRY_SINGLE(metaExif, MTK_3A_EXIF_DBGINFO_SDINFO_DATA, dbgShd);
                UPDATE_ENTRY_SINGLE(metaExif, MTK_3A_EXIF_DBGINFO_ISP_DATA, dbgIspP1);
            }

#if CAM3_STEREO_FEATURE_EN
            // debug info for N3D
            if(pAllResult->vecDbgN3DInfo.size())
            {
                CAM_LOGD_IF(fgLogCVT, "[%s] DebugInfo #(%d) for N3D", __FUNCTION__, pHALResult->i4FrmId);
                UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_N3D_EXIF_DBGINFO_KEY, DEBUG_EXIF_MID_CAM_N3D);
                IMetadata::Memory dbgN3D;
                dbgN3D.appendVector(pAllResult->vecDbgN3DInfo);
                UPDATE_ENTRY_SINGLE(metaExif, MTK_N3D_EXIF_DBGINFO_DATA, dbgN3D);
            }
#endif

            UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_3A_EXIF_METADATA, metaExif);
            UPDATE_ENTRY_SINGLE<MUINT8>(pMetaResult->rMetaResult.halMeta, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);

            UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_ANALOG_GAIN, (MINT32)pISPResult->rCamInfo.rAEInfo.u4AfeGain);
            UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_AWB_RGAIN, (MINT32)pAWBResult->i4AwbGain[0]);
            UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_AWB_GGAIN, (MINT32)pAWBResult->i4AwbGain[1]);
            UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_AWB_BGAIN, (MINT32)pAWBResult->i4AwbGain[2]);
        }
    }
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(print2AInfo);
    // log of control
    MINT32 rRgn[5] = {};
    MUINT8 u1Mode;
    if (GET_ENTRY_ARRAY(pMetaResult->rMetaResult.appMeta, MTK_CONTROL_AE_REGIONS, rRgn, 5))
    {
        CAM_LOGD_IF(fgLogCVT, "[%s] MTK_CONTROL_AE_REGIONS(%d,%d,%d,%d,%d)", __FUNCTION__,
            rRgn[0], rRgn[1], rRgn[2], rRgn[3], rRgn[4]);
    }

    if (QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_COLOR_CORRECTION_ABERRATION_MODE, u1Mode))
    {
        CAM_LOGD_IF(fgLogCVT, "[%s] MTK_COLOR_CORRECTION_ABERRATION_MODE(%d)", __FUNCTION__, u1Mode);
    }
    if (QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_NOISE_REDUCTION_MODE, u1Mode))
    {
        CAM_LOGD_IF(fgLogCVT, "[%s] MTK_NOISE_REDUCTION_MODE(%d)", __FUNCTION__, u1Mode);
    }
    double noise_profile[8];
    if (GET_ENTRY_ARRAY(pMetaResult->rMetaResult.appMeta, MTK_SENSOR_NOISE_PROFILE, noise_profile, 8))
    {
        CAM_LOGD_IF(fgLogCVT, "[%s] MTK_SENSOR_NOISE_PROFILE (%f,%f,%f,%f,%f,%f,%f,%f)", __FUNCTION__,
            noise_profile[0], noise_profile[1], noise_profile[2], noise_profile[3],
            noise_profile[4], noise_profile[5], noise_profile[6], noise_profile[7]);
    }
    if (QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_HOT_PIXEL_MODE, u1Mode))
    {
        CAM_LOGD_IF(fgLogCVT, "[%s] MTK_HOT_PIXEL_MODE(%d)", __FUNCTION__, u1Mode);
    }
    if (QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_STATISTICS_HOT_PIXEL_MAP_MODE, u1Mode))
    {
        CAM_LOGD_IF(fgLogCVT, "[%s] MTK_STATISTICS_HOT_PIXEL_MAP_MODE(%d)", __FUNCTION__, u1Mode);
    }
    AAA_TRACE_END_HAL;


    {
        Mutex::Autolock Vec_lock(pAllResult->LockVecResult);
        AAA_TRACE_HAL(convert2AROI);
        CAM_LOGD_IF(fgLogArea, "[%s] MTK_3A_FEATURE_AE_ROI", __FUNCTION__);
        convert3AROI(pAllResult->vecAEROI, pMetaResult, MTK_3A_FEATURE_AE_ROI);
    }

    {
        Mutex::Autolock Vec_lock(pAllResult->LockVecResult);
        CAM_LOGD_IF(fgLogArea, "[%s] MTK_3A_FEATURE_AWB_ROI", __FUNCTION__);
        convert3AROI(pAllResult->vecAWBROI, pMetaResult, MTK_3A_FEATURE_AWB_ROI);
        AAA_TRACE_END_HAL;
    }

    AAA_TRACE_HAL(convertStereoROI);
    // stereo warning message
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_STEREO_FEATURE_WARNING, pHALResult->i4StereoWarning);
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(updateImportantInfo42A);
    // update Important Info as pre-info
    m_r3AImportantInfo.u1AeState = pAEResult->u1AeState;
    m_r3AImportantInfo.u1AwbState = pAWBResult->u1AwbState;
    m_r3AImportantInfo.u1FlashState = pFLASHResult->u1FlashState;
    m_r3AImportantInfo.i8SensorExposureTime = pAEResult->i8SensorExposureTime;
    m_r3AImportantInfo.i8SensorFrameDuration = pAEResult->i8SensorFrameDuration;
    m_r3AImportantInfo.i4SensorSensitivity = pAEResult->i4SensorSensitivity;
    QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_CONTROL_CAPTURE_INTENT, u1CapIntent);
    QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_CONTROL_AE_PRECAPTURE_TRIGGER, u1AePrecapTrig);
    m_r3AImportantInfo.u1CapIntent = u1CapIntent;
    m_r3AImportantInfo.u1AePrecapTrig = u1AePrecapTrig;
    m_r3AImportantInfo.u1AWBMode = u1AWBMode;
    m_r3AImportantInfo.u1ColorCorrectMode = pISPResult->rCamInfo.eColorCorrectionMode;
    AAA_TRACE_END_HAL;

    pMetaResult->i4IsConverted |= E_2A_CONVERT;


    CAM_LOGD_IF(fgLogCVT, "[%s]- IsConverted(%d)", __FUNCTION__, pMetaResult->i4IsConverted);
    return MFALSE;
}

IResultPool*
IResultPool::
getInstance(MINT32 i4SensorDevId)
{
    return ResultPoolImp::getInstance(i4SensorDevId);
}

IResultPool*
ResultPoolImp::
getInstance(MINT32 i4SensorDevId)
{
    GET_PROP("vendor.debug.resultpool.log", 0, m_u4LogEn);
    auto i4SensorIdx = mapSensorDevToIdx(i4SensorDevId);
    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        MY_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
        return nullptr;
    }

    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ResultPoolImp>(i4SensorIdx);
    } );

    return rSingleton.instance.get();
}

ResultPoolImp::
ResultPoolImp(MINT32 i4SensorIdx)
    : m_pThreadResultPool(NULL)
    , m_rResultPool(NULL)
    , m_rMetaResultPool(NULL)
    , m_u4Capacity(Capacity)
    , m_r3AImportantInfo()
    , m_i4SubsampleCount(SubsampleCount)
    , m_LockHistory()
    , m_pLSCGain(NULL)
    , m_LockLastInfo()
    , m_i4CurIndex(0)
    , m_LockCurIndex()
{
    CAM_LOGD("[%s] Constructor initial, Capacity(%d) defaultMetaMagicNum:%d", __FUNCTION__, m_u4Capacity, m_defaultMeta.rMetaResult.MagicNum);
    m_rBasicCfg.u4SensorDev = mapSensorIdxToDev(i4SensorIdx);
    //===== Clear Last Info =====
    ::memset(&m_vLastInfo, 0, sizeof(LastInfo_T));
}

ResultPoolImp::~ResultPoolImp()
{
    CAM_LOGD("[%s] Destructor", __FUNCTION__);
}

MVOID
ResultPoolImp::
init()
{

    CAM_LOGD("[%s]+ i4SensorDev:%d", __FUNCTION__, m_rBasicCfg.u4SensorDev);

    m_pLSCGain = (MUINT16**) malloc(3*sizeof(MUINT16*));

    //===== Create Thread of resultPool =====
    if(m_pThreadResultPool == NULL)
    {
        m_pThreadResultPool = ThreadResultPool::createInstance(m_rBasicCfg.u4SensorDev);
        if (m_pThreadResultPool)
        {
            m_pThreadResultPool->init();
        }
        else
            CAM_LOGE("[%s] ThreadResultPool created fail!", __FUNCTION__);
    }

    //===== Add basic module enum, because it need to convert to metadata =====
    for(MINT32 i4EnumIdx = 0; i4EnumIdx < E_NUM_RESULTTOMETA; i4EnumIdx++)
    {
        if(i4EnumIdx != E_AF_RESULTTOMETA)
            m_i4ValidateEnumList4Meta.push_back(i4EnumIdx);
    }

    //===== Clear request magic number vector =====
    {
        Mutex::Autolock lock(m_LockHistory);
        m_vecHistory.clear();
    }

    //===== allocate metadata & result pool =====
    allocateResultPool();
    CAM_LOGD("[%s]- ThreadResultPool Addr:%p, ValidateEnum Size:%d", __FUNCTION__, m_pThreadResultPool, m_i4ValidateEnumList4Meta.size());
}

MVOID
ResultPoolImp::
config(const BASIC_CFG_INFO_T& rBasicCfg)
{
    CAM_LOGD("[%s]+ SensorIdx:%d, SensorDev:%d, SensorMode:%d", __FUNCTION__, rBasicCfg.i4SensorIdx, rBasicCfg.u4SensorDev, rBasicCfg.i4SensorMode);
    m_rBasicCfg = rBasicCfg;
    CAM_LOGD("[%s]- ", __FUNCTION__);
}

MVOID
ResultPoolImp::
uninit()
{
    CAM_LOGD("[%s]+ i4SensorDev:%d", __FUNCTION__, m_rBasicCfg.u4SensorDev);

    free(m_pLSCGain);
    //===== destroy Thread of resultPool =====
    if(m_pThreadResultPool)
    {
        m_pThreadResultPool->destroyInstance();
        m_pThreadResultPool = NULL;
    }

    //===== clear basic module enum =====
    m_i4ValidateEnumList4Meta.clear();

    //===== Clear request magic number vector =====
    {
        Mutex::Autolock lock(m_LockHistory);
        m_vecHistory.clear();
    }

    //===== free result pool =====
    freeResultPool();

    CAM_LOGD("[%s]-", __FUNCTION__);
}

MVOID
ResultPoolImp::
allocateResultPool()
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_OTHER) ? MTRUE: MFALSE;

    CAM_LOGD("[%s]+", __FUNCTION__);

    //===== Allocate Metadata ResulPool buffer =====
    if(m_rMetaResultPool == NULL)
        m_rMetaResultPool = new AllMetaResult_T[m_u4Capacity];
    else
        CAM_LOGD("[%s] Meta ResultPool exist", __FUNCTION__);
    //===== Debug for Metadata ResulPool buffer =====
    if(m_rMetaResultPool)
    {
        CAM_LOGD_IF(fgLog, "[%s] Create Meta ResultPool Success", __FUNCTION__);
        for(MUINT32 i4Idx = 0; i4Idx < m_u4Capacity; i4Idx++)
            CAM_LOGD_IF(fgLog, "[%s] MetaResultPool[%d]:%p", __FUNCTION__, i4Idx, &m_rMetaResultPool[i4Idx]);
    }
    else
        CAM_LOGE("[%s] Create Meta ResultPool fail", __FUNCTION__);

    //===== Allocate ResultPool buffer =====
    if(m_rResultPool == NULL)
        m_rResultPool = new AllResult_T[m_u4Capacity];
    else
        CAM_LOGD("[%s] ResultPool exist", __FUNCTION__);

    if(m_rResultPool)
    {
        CAM_LOGD_IF(fgLog, "[%s] Create ResultPool Success", __FUNCTION__);
        for(MUINT32 i4Idx = 0; i4Idx < m_u4Capacity; i4Idx++)
        {
            for(MINT32 i4EnumIdx = 0; i4EnumIdx < E_NUM_RESULT_END; i4EnumIdx++)
            {
                CAM_LOGD_IF(fgLog, "[%s] (ResultPool[%d], ModuleResultAddr[%d]) = (%p, %p)", __FUNCTION__, i4Idx, i4EnumIdx, &m_rResultPool[i4Idx], m_rResultPool[i4Idx].ModuleResultAddr[i4EnumIdx]);
                if(m_rResultPool[i4Idx].ModuleResultAddr[i4EnumIdx] == NULL)
                {
                    MINT32 size = enumToSize(static_cast<E_PARTIAL_RESULT_OF_MODULE_T>(i4EnumIdx));
                    CAM_LOGD_IF(fgLog, "[%s] Module:%d -> Size:%d", __FUNCTION__, i4EnumIdx, size);
                    if(size != -1)
                        m_rResultPool[i4Idx].ModuleResultAddr[i4EnumIdx] = new Entry(LOG_TAG, size);

                    if(m_rResultPool[i4Idx].ModuleResultAddr[i4EnumIdx] != NULL)
                        CAM_LOGD_IF(fgLog, "[%s] Create Modules Result Buffer Addr Success:%p", __FUNCTION__, m_rResultPool[i4Idx].ModuleResultAddr[i4EnumIdx]);
                    else
                        CAM_LOGD_IF(fgLog, "[%s] Create Modules Result Buffer Addr Fail - EnumIdx(%d)", __FUNCTION__, i4EnumIdx);
                }
            }
            // resize vector size
            {
                Mutex::Autolock Vec_lock(m_rResultPool[i4Idx].LockVecResult);
                if(m_rResultPool[i4Idx].vecExifInfo.size()==0)
                    m_rResultPool[i4Idx].vecExifInfo.resize(1);
                if(m_rResultPool[i4Idx].vecDbg3AInfo.size()==0)
                    m_rResultPool[i4Idx].vecDbg3AInfo.resize(sizeof(AAA_DEBUG_INFO1_T));
                if(m_rResultPool[i4Idx].vecDbgIspInfo.size()==0)
                    m_rResultPool[i4Idx].vecDbgIspInfo.resize(sizeof(AAA_DEBUG_INFO2_T));
                if(m_rResultPool[i4Idx].vecDbgShadTbl.size()==0)
                    m_rResultPool[i4Idx].vecDbgShadTbl.resize(sizeof(DEBUG_SHAD_ARRAY_INFO_T));
            }
        }
    }
    else
        CAM_LOGE("[%s] Ceate ResultPool Fail", __FUNCTION__);

    CAM_LOGD("[%s]-", __FUNCTION__);
}

MVOID
ResultPoolImp::
freeResultPool()
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_OTHER) ? MTRUE: MFALSE;

    CAM_LOGD("[%s]+", __FUNCTION__);

    //===== Delete Metadata ResulPool buffer =====
    if(m_rMetaResultPool)
    {
        CAM_LOGD_IF(fgLog, "[%s] Delete Meta ResultPool", __FUNCTION__);
        delete[] m_rMetaResultPool;
        m_rMetaResultPool = NULL;
    }
    else
        CAM_LOGD("[%s] Meta ResultPool already free", __FUNCTION__);

    //===== Delete ResulPool buffer =====
    if(m_rResultPool)
    {
        CAM_LOGD_IF(fgLog, "[%s] Delete ResultPool", __FUNCTION__);
        for(MUINT32 i4Idx = 0; i4Idx < m_u4Capacity; i4Idx++)
        {
            for(MINT32 i4EnumIdx = 0; i4EnumIdx < E_NUM_RESULT_END; i4EnumIdx++)
            {
                if(m_rResultPool[i4Idx].ModuleResultAddr[i4EnumIdx])
                {
                    CAM_LOGD_IF(fgLog, "[%s] (ResultPool[%d], ModuleResultAddr[%d]) = (%p, %p)", __FUNCTION__, i4Idx, i4EnumIdx, &m_rResultPool[i4Idx], m_rResultPool[i4Idx].ModuleResultAddr[i4EnumIdx]);
                    delete m_rResultPool[i4Idx].ModuleResultAddr[i4EnumIdx];
                    m_rResultPool[i4Idx].ModuleResultAddr[i4EnumIdx] = NULL;
                }
            }
        }
        delete[] m_rResultPool;
        m_rResultPool = NULL;
    }
    else
        CAM_LOGD("[%s] ResultPool already free", __FUNCTION__);

    CAM_LOGD("[%s]-", __FUNCTION__);
}

MVOID
ResultPoolImp::
clearAllResultPool()
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_OTHER) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s]+", __FUNCTION__);

    //===== preview start to reset Current index ====
    m_i4CurIndex = 0;

    //===== Clear request magic number vector =====
    {
        Mutex::Autolock lock(m_LockHistory);
        m_vecHistory.clear();
    }

    //===== Clear Metadata ResulPool& ResulPool buffer =====
    if(m_rMetaResultPool && m_rResultPool)
    {
        for(MUINT32 i4Idx = 0; i4Idx < m_u4Capacity; i4Idx++)
        {
            resetResultPool(i4Idx);
        }
    }

    CAM_LOGD_IF(fgLog, "[%s]-", __FUNCTION__);
}

MVOID
ResultPoolImp::
resetResultPool(MINT32 i4Indx)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_OTHER) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s]+ Indx:%d", __FUNCTION__, i4Indx);

    //===== Clear Metadata ResulPoolmember =====
    m_rMetaResultPool[i4Indx].rMetaResult.Dummy = 0;
    m_rMetaResultPool[i4Indx].rMetaResult.MagicNum = -1;
    m_rMetaResultPool[i4Indx].i4IsConverted = E_NON_CONVERT;
    m_rResultPool[i4Indx].rResultCfg.i4StatisticMagic = InvalidValue;

    //===== Clear ResulPool member =====
    //m_rResultPool[i4Indx].rResultCfg.strUserName.clear();
    for(MINT32 i4EnumIdx = 0; i4EnumIdx < E_NUM_RESULT_END; i4EnumIdx++)
    {
        if(m_rResultPool[i4Indx].ModuleResultAddr[i4EnumIdx] != NULL && i4EnumIdx != E_NUM_RESULTTOMETA && i4EnumIdx != E_NUM_RESULT_END)
        {
            m_rResultPool[i4Indx].ModuleResultAddr[i4EnumIdx]->reset();
        }
    }

    {
        Mutex::Autolock Vec_lock(m_rResultPool[i4Indx].LockVecResult);
        m_rResultPool[i4Indx].vecExifInfo.clear();
        m_rResultPool[i4Indx].vecColorCorrectMat.clear();
        m_rResultPool[i4Indx].vecDbg3AInfo.clear();
        m_rResultPool[i4Indx].vecDbgShadTbl.clear();
        m_rResultPool[i4Indx].vecDbgIspInfo.clear();
        m_rResultPool[i4Indx].vecDbgN3DInfo.clear();
        m_rResultPool[i4Indx].vecAFROI.clear();
        m_rResultPool[i4Indx].vecAEROI.clear();
        m_rResultPool[i4Indx].vecAWBROI.clear();
        m_rResultPool[i4Indx].vecLscData.clear();
    }
#if 0
    // Reset MetaResultPool member
    m_rMetaResultPool[i4Indx].rMetaResult.Dummy = 0;
    m_rMetaResultPool[i4Indx].rMetaResult.MagicNum = -1;

    // Reset ResultPool member
    //::memset( m_rResultPool[i4Indx].rResultCfg.i4HistoryReqMagic, InvalidIndex, sizeof(m_rResultPool[i4Idx].rResultCfg.i4HistoryReqMagic)*3);
    //m_rResultPool[i4Indx].rResultCfg.i4HistoryReqMagic[0] = 0;
    //m_rResultPool[i4Indx].rResultCfg.i4HistoryReqMagic[1] = 0;
    //m_rResultPool[i4Indx].rResultCfg.i4HistoryReqMagic[2] = 0;

    m_rResultPool[i4Indx].rResultCfg.i4ReqMagic = InvalidValue;
    m_rResultPool[i4Indx].rResultCfg.i4StatisticMagic = InvalidValue;
    //m_rResultPool[i4Indx].rResultCfg.bIsNeedConvertToMeta = 1;
    m_rResultPool[i4Indx].rResultCfg.strUserName.clear();

    m_rResultPool[i4Indx].rOld3AInfo.i4Magic4SMVR[0] = InvalidValue;
    m_rResultPool[i4Indx].rOld3AInfo.i4Magic4SMVR[1] = InvalidValue;
    m_rResultPool[i4Indx].rOld3AInfo.i4Magic4SMVR[2] = InvalidValue;
    m_rResultPool[i4Indx].rOld3AInfo.i4Magic4SMVR[3] = InvalidValue;
    m_rResultPool[i4Indx].rOld3AInfo.i4SensorSensitivity = 0;
    m_rResultPool[i4Indx].rOld3AInfo.i8SensorExposureTime = 0;
    m_rResultPool[i4Indx].rOld3AInfo.u1AeState = 0;
    m_rResultPool[i4Indx].rOld3AInfo.u1AfState = 0;

    for(MINT32 i4EnumIdx = 0; i4EnumIdx < E_NUM_RESULT_END; i4EnumIdx++)
    {
        if(m_rResultPool[i4Indx].ModuleResultAddr[i4EnumIdx])
            m_rResultPool[i4Indx].ModuleResultAddr[i4EnumIdx]->reset();
    }
    // exif
    m_rResultPool[i4Indx].vecExifInfo.clear();
    m_rResultPool[i4Indx].vecDbg3AInfo.clear();
    m_rResultPool[i4Indx].vecDbgShadTbl.clear();
    m_rResultPool[i4Indx].vecDbgIspInfo.clear();
    m_rResultPool[i4Indx].vecDbgN3DInfo.clear();

    // AE AF AWB ROI for APP META
    m_rResultPool[i4Indx].vecAEROI.clear();
    m_rResultPool[i4Indx].vecAFROI.clear();
    m_rResultPool[i4Indx].vecAWBROI.clear();

    // focus area result
    m_rResultPool[i4Indx].vecFocusAreaPos.clear();
    m_rResultPool[i4Indx].vecFocusAreaRes.clear();
    //ISP
    m_rResultPool[i4Indx].vecColorCorrectMat.clear();
    //LSC
    m_rResultPool[i4Indx].vecLscData.clear();
#endif

    CAM_LOGD_IF(fgLog, "[%s]-", __FUNCTION__);
}

MVOID
ResultPoolImp::
clearOldestResultPool(MINT32 i4ReqMagic, MINT32 i4Index)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_OTHER) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s]+ (ReqMagic, SubsampleCount) = (%d, %d)", __FUNCTION__, i4ReqMagic, m_i4SubsampleCount);

    if(m_i4SubsampleCount > 1) // For slow motion
    {
        // Only 32 of buffer to use, because cacacity has garbage buffer.
        resetResultPool( i4Index );
    }
    else // For normal
    {
        if(i4ReqMagic == 0 && i4Index == ResultDummyIndex) // clear garbage buffer.
        {
            CAM_LOGW("[%s] garbage buffer (ReqMagic, Index) = (%d ,%d)", __FUNCTION__, i4ReqMagic, i4Index);
            resetResultPool( ResultDummyIndex );
        }
        else // Only 8 of buffer to use, because cacacity has garbage buffer.
            resetResultPool( i4Index );
    }
    CAM_LOGD_IF(fgLog, "[%s]-", __FUNCTION__);
}

const MVOID*
ResultPoolImp::
getResult(MINT32 i4ReqMagic, E_PARTIAL_RESULT_OF_MODULE_T eModule)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_GET_1) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s]+ (ReqMagic, eModule) = (%d, %d)", __FUNCTION__, i4ReqMagic, eModule);

    //===== ReqMagic to Index =====
    MINT32 i4Index = magic2IndexRingBuffer(i4ReqMagic);
    MBOOL bIsValidate = MFALSE;

    if(i4Index == -1)
    {
        CAM_LOGD_IF(fgLog, "[%s]- Index is illegal value(%d)", __FUNCTION__, i4Index);
        return NULL;
    }

    //===== ResultPool ReqMagic need to equal input ReqMagic, if doesn't equal return NULL =====
    if(m_rResultPool[i4Index].rResultCfg.i4ReqMagic != i4ReqMagic)
    {
        CAM_LOGD_IF(fgLog, "[%s]- ResultPool ReqMagic doesn't mapping input ReqMagic (%d, %d, %d)", __FUNCTION__, m_rResultPool[i4Index].rResultCfg.i4ReqMagic, i4ReqMagic, i4Index);
        return NULL;
    }

    //===== Return validate module Addr, if doesn't validate return NULL =====
    if(m_rResultPool[i4Index].ModuleResultAddr[eModule])
    {
        bIsValidate = m_rResultPool[i4Index].ModuleResultAddr[eModule]->isValidate();
        CAM_LOGD_IF(fgLog, "[%s]- bIsValidate:%d", __FUNCTION__, bIsValidate);

        if(bIsValidate)
            return m_rResultPool[i4Index].ModuleResultAddr[eModule]->read();
    }
    CAM_LOGD_IF(fgLog, "[%s]- Module doesn't validate %d", __FUNCTION__, bIsValidate);
    return NULL;
}

MINT32
ResultPoolImp::
updateResult(std::string strUserName, MINT32 i4ReqMagic, E_PARTIAL_RESULT_OF_MODULE_T eModule, const MVOID* pData) // pData = xxxResult_T
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_SET_1) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s]+ (ReqMagic, eModule, UserName) = (%d, %d, %s)", __FUNCTION__, i4ReqMagic, eModule, strUserName.c_str());

    //===== ReqMagic to Index =====
    MINT32 i4Index = magic2IndexRingBuffer(i4ReqMagic);

    if(i4Index == -1)
    {
        CAM_LOGW("[%s]- Index is illegal value(%d)", __FUNCTION__, i4Index);
        return NULL;
    }

    //===== ResultPool ReqMagic need to equal input ReqMagic, if doesn't equal return NULL =====
    if(m_rResultPool[i4Index].rResultCfg.i4ReqMagic != i4ReqMagic)
    {
        CAM_LOGD_IF(fgLog, "[%s]- ResultPool ReqMagic doesn't mapping input ReqMagic (%d, %d, %d)", __FUNCTION__, m_rResultPool[i4Index].rResultCfg.i4ReqMagic, i4ReqMagic, i4Index);
        return MFALSE;
    }

    //===== Update module =====
    if(m_rResultPool[i4Index].ModuleResultAddr[eModule])
    {
        // Stash debug info //racing condition
        //sprintf(dbgInfo, "%s_%d ", strUserName.c_str(), eModule);
        //m_rResultPool[i4Index].rResultCfg.strUserName.append(dbgInfo);

        // write data
        MINT32 size = enumToSize(static_cast<E_PARTIAL_RESULT_OF_MODULE_T>(eModule));

        m_rResultPool[i4Index].ModuleResultAddr[eModule]->write(size, pData);
        CAM_LOGD_IF(fgLog, "[%s]- Module:%d -> size:%d", __FUNCTION__,eModule, size);
        return MTRUE;
    }

    CAM_LOGD_IF(fgLog, "[%s]- ", __FUNCTION__);
    return MFALSE;
}

AllResult_T*
ResultPoolImp::
getAllResult(MINT32 i4ReqMagic)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_GET_1) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s]+ ReqMagic:%d", __FUNCTION__, i4ReqMagic);

    //===== ReqMagic to Index =====
    MINT32 i4Index = magic2IndexRingBuffer(i4ReqMagic);

    if(i4Index == -1)
    {
        CAM_LOGW("[%s]- Index is illegal value(%d)", __FUNCTION__, i4Index);
        return NULL;
    }

    //===== ResultPool ReqMagic need to equal input ReqMagic, if doesn't equal return NULL =====
    if(m_rResultPool[i4Index].rResultCfg.i4ReqMagic != i4ReqMagic)
    {
        CAM_LOGD_IF(fgLog, "[%s]- ResultPool ReqMagic doesn't mapping input ReqMagic (%d, %d, %d)", __FUNCTION__, m_rResultPool[i4Index].rResultCfg.i4ReqMagic, i4ReqMagic, i4Index);
        return NULL;
    }

    CAM_LOGD_IF(fgLog, "[%s]- (m_rResultPool[%d], ReqMagic) = (%p, %d)", __FUNCTION__, i4Index, &m_rResultPool[i4Index], i4ReqMagic);
    return &m_rResultPool[i4Index];
}

MINT32
ResultPoolImp::
updateAllResult(std::string strUserName, MINT32 i4ReqMagic, const AllResult_T* pData) //pData = m_rResultPool
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_SET_1) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s]+ (ReqMagic, UserName) = (%d, %s)", __FUNCTION__, i4ReqMagic, strUserName.c_str());

    MINT32 i4Ret = MTRUE;

    for(MINT32 j = 0; j < E_NUM_RESULTTOMETA; j++)
    {
        MINT32 i4ResultRet = updateResult(strUserName, i4ReqMagic, static_cast<E_PARTIAL_RESULT_OF_MODULE_T>(j), pData->ModuleResultAddr[j]->read());
        i4Ret &= i4ResultRet;
    }

    CAM_LOGD_IF(fgLog, "[%s]- Ret:%d", __FUNCTION__, i4Ret);
    return i4Ret;
}

MINT32
ResultPoolImp::
updateCapacity(MUINT32 u4Capacity, MINT32 i4SubsampleCount)
{
    CAM_LOGD("[%s]+ (In-Capacity, Local-Capacity, SubsampleCount) = (%d, %d, %d)", __FUNCTION__, u4Capacity, m_u4Capacity, i4SubsampleCount);

    //===== normal change to slow motio, re-allocate resultPool buffer =====
    if(u4Capacity != m_u4Capacity)
    {
        m_u4Capacity = u4Capacity;
        m_i4SubsampleCount = i4SubsampleCount;
        freeResultPool();
        allocateResultPool();
        CAM_LOGD("[%s]- SubsampleCount(%d)", __FUNCTION__, m_i4SubsampleCount);
        return MTRUE;
    }
    CAM_LOGD("[%s]- SubsampleCount(%d)", __FUNCTION__, m_i4SubsampleCount);
    return MFALSE;
}

MINT32
ResultPoolImp::
isValidate(MINT32 i4ReqMagic)
{
    CAM_LOGD_IF(m_u4LogEn, "[%s]+ (ReqMagic, SensorDev) = (%d, %d)", __FUNCTION__, i4ReqMagic, m_rBasicCfg.u4SensorDev);

    //===== ReqMagic to Index =====
    MINT32 i4Index = magic2IndexRingBuffer(i4ReqMagic);
    MINT32 i4Validate = MFALSE;

    if(i4Index == -1)
    {
        CAM_LOGW("[%s]- Index is illegal value(%d)", __FUNCTION__, i4Index);
        return NULL;
    }

    //===== ResultPool ReqMagic need to equal input ReqMagic, if doesn't equal return NULL =====
    if(m_rResultPool[i4Index].rResultCfg.i4ReqMagic != i4ReqMagic)
    {
        CAM_LOGD_IF(m_u4LogEn, "[%s]- ResultPool ReqMagic doesn't mapping input ReqMagic (%d, %d, %d)", __FUNCTION__, m_rResultPool[i4Index].rResultCfg.i4ReqMagic, i4ReqMagic, i4Index);
        return (-1);
    }

    //===== Check basic module enum validate or not =====
    List<MINT32>::iterator it = m_i4ValidateEnumList4Meta.begin();
    for (; it != m_i4ValidateEnumList4Meta.end(); it++)
    {
        MINT32 &i4EnumIndex = (*it);
        if(m_rResultPool[i4Index].ModuleResultAddr[i4EnumIndex])
        {
            i4Validate = m_rResultPool[i4Index].ModuleResultAddr[i4EnumIndex]->isValidate();
            if(i4Validate == MFALSE)
                break;
        }
    }

    CAM_LOGD_IF(m_u4LogEn, "[%s]- Validate:%d", __FUNCTION__, i4Validate);
    return i4Validate;
}

MINT32
ResultPoolImp::
isValidateCur(MINT32 i4SttMagic)
{
    CAM_LOGD_IF(m_u4LogEn, "[%s]+ (SttMagic, SensorDev) = (%d, %d)", __FUNCTION__, i4SttMagic, m_rBasicCfg.u4SensorDev);

    MINT32 i4Validate = MFALSE;
    MINT32 i4Index = -1;

    //===== Find SttMagic index =====
    for(MUINT32 i = 0; i < m_u4Capacity; i++)
    {
        if(m_rResultPool[i].rResultCfg.i4StatisticMagic == i4SttMagic)
        {
            i4Index = i;
            CAM_LOGD("[%s] (ResultSttMagic, Index , SttMagic) = (%d, %d, %d)", __FUNCTION__, m_rResultPool[i].rResultCfg.i4StatisticMagic, i4Index, i4SttMagic);
        }
        //else
        //    CAM_LOGE("[%s]- ResultPool SttMagic doesn't mapping input SttMagic (%d, %d)", __FUNCTION__, m_rResultPool[i].rResultCfg.i4StatisticMagic, i4SttMagic);
    }

    //===== Check basic module enum validate or not =====
    if(i4Index != -1)
    {
        List<MINT32>::iterator it = m_i4ValidateEnumList4Meta.begin();
        for (; it != m_i4ValidateEnumList4Meta.end(); it++)
        {
            MINT32 &i4EnumIndex = (*it);
            if(m_rResultPool[i4Index].ModuleResultAddr[i4EnumIndex])
            {
                i4Validate = m_rResultPool[i4Index].ModuleResultAddr[i4EnumIndex]->isValidate();
                if(i4Validate == MFALSE)
                    break;
            }
        }
    }

    CAM_LOGD_IF(m_u4LogEn, "[%s] - Validate:%d Index:%d", __FUNCTION__, i4Validate, i4Index);
    return i4Validate;
}

MetaSet_T*
ResultPoolImp::
getValidateMetadata(MINT32 i4ReqMagic)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_SET_0) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s]+ ReqMagic:%d", __FUNCTION__, i4ReqMagic);

    //===== ReqMagic to Index =====
    MINT32 i4Index = getValidateIndex(i4ReqMagic);

    Mutex::Autolock lock(m_rMetaResultPool[i4Index].LockMetaResult);

    clearOldestResultPool(i4ReqMagic, i4Index);

    //===== update Requst magicNum to ResultPool =====
    m_rResultPool[i4Index].rResultCfg.i4ReqMagic = i4ReqMagic;

    CAM_LOGD_IF(fgLog, "[%s]- Index:%d", __FUNCTION__, i4Index);
    return &m_rMetaResultPool[i4Index].rMetaResult;
}

AllMetaResult_T*
ResultPoolImp::
getMetadata(MINT32 i4ReqMagic)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_GET_0) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s]+ ReqMagic:%d", __FUNCTION__, i4ReqMagic);

    //===== ReqMagic to Index =====
    MINT32 i4Index = magic2IndexRingBuffer(i4ReqMagic);

    if(i4Index == -1)
        return NULL;

    Mutex::Autolock lock(m_rMetaResultPool[i4Index].LockMetaResult);

    //===== ResultPool ReqMagic need to equal input ReqMagic, if doesn't equal return NULL =====
    if(m_rMetaResultPool[i4Index].rMetaResult.MagicNum != i4ReqMagic)
    {
        CAM_LOGD_IF(fgLog, "[%s]- ResultPool ReqMagic doesn't mapping input ReqMagic (%d, %d, %d)", __FUNCTION__, m_rMetaResultPool[i4Index].rMetaResult.MagicNum, i4ReqMagic, i4Index);
        return NULL;
    }

    CAM_LOGD_IF(fgLog, "[%s] (m_rMetaResultPool[%d].MagicNum, Addr) = (%d, %p)", __FUNCTION__, i4Index, m_rMetaResultPool[i4Index].rMetaResult.MagicNum, &m_rMetaResultPool[i4Index]);
    return &(m_rMetaResultPool[i4Index]);
}

MVOID
ResultPoolImp::
updateHistory(MINT32 i4ReqMagic, MINT32 Size)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_SET_0) ? MTRUE: MFALSE;

    Mutex::Autolock lock(m_LockHistory);

    m_vecHistory.push_back(i4ReqMagic);

    if(m_vecHistory.size() > static_cast<MUINT32>(Size))
        m_vecHistory.erase(m_vecHistory.begin());

    //===== For Debug =====
    for(MUINT32 i = 0; i < m_vecHistory.size(); i++)
        CAM_LOGD_IF(fgLog,"[%s]History = (%d, %d, %d, %d)", __FUNCTION__, m_vecHistory.size(), m_vecHistory[0], m_vecHistory[1], m_vecHistory[2]);
}

MINT32
ResultPoolImp::
getHistory(MINT32* pHistory)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_GET_0) ? MTRUE: MFALSE;

    Mutex::Autolock lock(m_LockHistory);

    CAM_LOGD_IF(fgLog,"[%s]m_vecHistory.empty() %d", __FUNCTION__, m_vecHistory.empty());

    if(m_vecHistory.size() == 0)
    {
        CAM_LOGD("[%s]m_vecHistory.size() %d", __FUNCTION__, m_vecHistory.size());
        return MFALSE;
    }

    //===== For Debug =====
    if(!m_vecHistory.empty())
    {
        for(MUINT32 i = 0; i < m_vecHistory.size(); i++)
            CAM_LOGD_IF(fgLog,"[%s]History = (%d, %d, %d, %d)", __FUNCTION__, m_vecHistory.size(), m_vecHistory[0], m_vecHistory[1], m_vecHistory[2]);
    }

    //===== Assigne to out buffer =====
    for(MUINT32 i = 0; i < m_vecHistory.size(); i++)
    {
        *pHistory = m_vecHistory[i];
        pHistory++;
    }
    return MTRUE;
}

MVOID
ResultPoolImp::
setMetaStaticInfo(const IMetadata& metaStaticInfo)
{
    CAM_LOGD("[%s]+ ", __FUNCTION__);
    m_metaStaticInfo = metaStaticInfo;
    CAM_LOGD("[%s]- ", __FUNCTION__);
}

IMetadata&
ResultPoolImp::
getMetaStaticInfo()
{
    CAM_LOGD("[%s] ", __FUNCTION__);
    return m_metaStaticInfo;
}

#if 0
MINT32
ResultPoolImp::
magic2Index(MINT32 i4ReqMagic)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_PF) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s]+ ReqMagic:%d", __FUNCTION__, i4ReqMagic);
    MINT32 i4Index = 0;

    //===== ReqMagic to Index =====
    if(m_i4SubsampleCount > 1) // For slow motion
    {
        i4Index = (i4ReqMagic % (m_u4Capacity - (m_i4SubsampleCount*ResultNum4Dummy) ));
    }
    else // For normal
    {
        if(i4ReqMagic == 0) // Garbage buffer
            i4Index = ResultDummyIndex;
        else
            i4Index = (i4ReqMagic % ResultNum);
    }

    CAM_LOGD_IF(fgLog, "[%s] - Final Index(%d)", __FUNCTION__, i4Index);

    return i4Index;
}
#endif

MINT32
ResultPoolImp::
getValidateIndex(MINT32 i4ReqMagic)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_PF) ? MTRUE: MFALSE;

    Mutex::Autolock lock(m_LockCurIndex);
    CAM_LOGD_IF(fgLog, "[%s]+ ReqMagic:%d, SubsampleCount:%d, m_i4CurIndex:%d", __FUNCTION__, i4ReqMagic, m_i4SubsampleCount, m_i4CurIndex);
    MINT32 i4Index = 0;

    //===== ReqMagic to Index =====
    if(m_i4SubsampleCount > 1) // For slow motion
    {
        m_i4CurIndex = ( (m_i4CurIndex+1) % (m_i4SubsampleCount*ResultNum) );
        i4Index = m_i4CurIndex;
    }
    else // For normal
    {

        m_i4CurIndex = ((m_i4CurIndex+1)%ResultNum);

        if(i4ReqMagic == 0) // Garbage buffer
            i4Index = ResultDummyIndex;
        else
            i4Index = m_i4CurIndex;
    }

    CAM_LOGD_IF(fgLog, "[%s] - Final Index(%d)", __FUNCTION__, i4Index);

    return i4Index;
}

MINT32
ResultPoolImp::
magic2IndexRingBuffer(MINT32 i4ReqMagic)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_PF) ? MTRUE: MFALSE;

    Mutex::Autolock lock(m_LockCurIndex);
    CAM_LOGD_IF(fgLog, "[%s]+ ReqMagic:%d, SubsampleCount:%d, m_i4CurIndex:%d", __FUNCTION__, i4ReqMagic, m_i4SubsampleCount, m_i4CurIndex);
    MINT32 i4Index = 0;
    MINT32 i4CurIndex = m_i4CurIndex;

    if(i4ReqMagic < 0 || m_rResultPool == NULL)
    {
        CAM_LOGE("[%s] ReqMagic is Negative:%d, SubsampleCount:%d, m_rResultPool(%p)", __FUNCTION__, i4ReqMagic, m_i4SubsampleCount, m_rResultPool);
        i4Index = -1;
        return i4Index;
    }
    //===== ReqMagic to Index =====
    if(m_i4SubsampleCount > 1) // For slow motion
    {
        MINT32 i4BufferSize = (m_i4SubsampleCount*ResultNum);
        i4Index = (i4CurIndex - 1 + i4BufferSize)%i4BufferSize;
        while(i4Index != i4CurIndex)
        {
            CAM_LOGD_IF(fgLog, "[%s] (i4Index, Local-i4CurIndex, Golbal-i4CurIndex) = (%d, %d, %d) MagicNum(%d) ", __FUNCTION__, i4Index, i4CurIndex, m_i4CurIndex, m_rMetaResultPool[i4Index].rMetaResult.MagicNum);
            if(m_rMetaResultPool[i4Index].rMetaResult.MagicNum == i4ReqMagic)
                break;
            i4Index = (i4Index - 1 + i4BufferSize)% i4BufferSize;
        }
        if(i4Index == i4CurIndex && m_rMetaResultPool[i4Index].rMetaResult.MagicNum != i4ReqMagic)
            i4Index = -1;
    }
    else // For normal
    {
        i4Index = (i4CurIndex - 1 + ResultNum)% ResultNum;
        while(i4Index != i4CurIndex)
        {
            CAM_LOGD_IF(fgLog, "[%s] (i4Index, Local-i4CurIndex, Golbal-i4CurIndex) = (%d, %d, %d) MagicNum(%d) ", __FUNCTION__, i4Index, i4CurIndex, m_i4CurIndex, m_rMetaResultPool[i4Index].rMetaResult.MagicNum);
            if(m_rMetaResultPool[i4Index].rMetaResult.MagicNum == i4ReqMagic)
                break;
            i4Index = (i4Index - 1 + ResultNum)% ResultNum;
        }
        if(i4Index == i4CurIndex && m_rMetaResultPool[i4Index].rMetaResult.MagicNum != i4ReqMagic)
            i4Index = -1;
    }

    CAM_LOGD_IF(fgLog, "[%s] - Final Index(%d)", __FUNCTION__, i4Index);

    return i4Index;
}


RESULT_CFG_T*
ResultPoolImp::
getResultCFG(MINT32 i4ReqMagic)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_GET_0) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s]+ ReqMagic:%d", __FUNCTION__, i4ReqMagic);

    //===== ReqMagic to Index =====
    MINT32 i4Index = magic2IndexRingBuffer(i4ReqMagic);

    if(i4Index == -1)
    {
        CAM_LOGW("[%s]- Index is illegal value(%d)", __FUNCTION__, i4Index);
        return NULL;
    }

    //===== ResultPool ReqMagic need to equal input ReqMagic, if doesn't equal return NULL =====
    if(m_rResultPool[i4Index].rResultCfg.i4ReqMagic != i4ReqMagic)
    {
        CAM_LOGD_IF(fgLog, "[%s]- ResultPool ReqMagic doesn't mapping input ReqMagic (%d, %d, %d)", __FUNCTION__, m_rResultPool[i4Index].rResultCfg.i4ReqMagic, i4ReqMagic, i4Index);
        return NULL;
    }

    CAM_LOGD_IF(fgLog, "[%s]- i4Index:%d", __FUNCTION__, i4Index);

    return &m_rResultPool[i4Index].rResultCfg;
}
MBOOL
ResultPoolImp::
updateResultCFG(MINT32 i4ReqMagic,MINT32 i4SttMagic)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_GET_0) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s]+ (ReqMagic, SttMagic) = (%d, %d)", __FUNCTION__, i4ReqMagic, i4SttMagic);

    //===== ReqMagic to Index =====
    MINT32 i4Index = magic2IndexRingBuffer(i4ReqMagic);

    if(i4Index == -1)
    {
        CAM_LOGW("[%s]- Index is illegal value(%d)", __FUNCTION__, i4Index);
        return NULL;
    }

    //===== ResultPool ReqMagic need to equal input ReqMagic, if doesn't equal return NULL =====
    if(m_rResultPool[i4Index].rResultCfg.i4ReqMagic != i4ReqMagic)
    {
        CAM_LOGD_IF(fgLog, "[%s]- ResultPool ReqMagic doesn't mapping input ReqMagic (%d, %d, %d)", __FUNCTION__, m_rResultPool[i4Index].rResultCfg.i4ReqMagic, i4ReqMagic, i4Index);
        return MFALSE;
    }
    else
        m_rResultPool[i4Index].rResultCfg.i4StatisticMagic = i4SttMagic;

    CAM_LOGD_IF(fgLog, "[%s]- Index:%d", __FUNCTION__, i4Index);

    return MTRUE;
}

MBOOL
ResultPoolImp::
updateEarlyCB(MINT32 i4ReqMagic, EARLY_CALL_BACK &rEarlyCB)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_GET_0) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s]+ ReqMagic:%d", __FUNCTION__, i4ReqMagic);

    //===== ReqMagic to Index =====
    MINT32 i4Index = magic2IndexRingBuffer(i4ReqMagic);

    if(i4Index == -1)
    {
        CAM_LOGW("[%s]- Index is illegal value(%d)", __FUNCTION__, i4Index);
        return NULL;
    }

    //===== ResultPool ReqMagic need to equal input ReqMagic, if doesn't equal return NULL =====
    if(m_rResultPool[i4Index].rResultCfg.i4ReqMagic != i4ReqMagic)
    {
        CAM_LOGD_IF(fgLog, "[%s]- ResultPool ReqMagic doesn't mapping input ReqMagic (%d, %d, %d)", __FUNCTION__, m_rResultPool[i4Index].rResultCfg.i4ReqMagic, i4ReqMagic, i4Index);
        return MFALSE;
    }
    else
    {
        //m_rResultPool[i4Index].rEarlyCB.u1AfState = rEarlyCB.u1AfState;
        ::memcpy(&m_rResultPool[i4Index].rEarlyCB, &rEarlyCB, sizeof(rEarlyCB));
    }

    CAM_LOGD_IF(fgLog, "[%s]- (Index, AfState, AeState, AeMode, FlashState, AeBvTrigger) = (%d, %d, %d, %d, %d, %d)", __FUNCTION__, i4Index, m_rResultPool[i4Index].rEarlyCB.u1AfState, m_rResultPool[i4Index].rEarlyCB.u1AeState,
                m_rResultPool[i4Index].rEarlyCB.u1AeMode, m_rResultPool[i4Index].rEarlyCB.u1FlashState, m_rResultPool[i4Index].rEarlyCB.fgAeBvTrigger);

    return MTRUE;
}

EARLY_CALL_BACK*
ResultPoolImp::
getEarlyCB(MINT32 i4ReqMagic)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_GET_0) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s]+ ReqMagic:%d", __FUNCTION__, i4ReqMagic);

    //===== ReqMagic to Index =====
    MINT32 i4Index = magic2IndexRingBuffer(i4ReqMagic);

    if(i4Index == -1)
    {
        CAM_LOGW("[%s]- Index is illegal value(%d)", __FUNCTION__, i4Index);
        return NULL;
    }

    //===== ResultPool ReqMagic need to equal input ReqMagic, if doesn't equal return NULL =====
    if(m_rResultPool[i4Index].rResultCfg.i4ReqMagic != i4ReqMagic)
    {
        CAM_LOGD_IF(fgLog, "[%s]- ResultPool ReqMagic doesn't mapping input ReqMagic (%d, %d, %d)", __FUNCTION__, m_rResultPool[i4Index].rResultCfg.i4ReqMagic, i4ReqMagic, i4Index);
        return NULL;
    }

    CAM_LOGD_IF(fgLog, "[%s]- (Index, AfState, AeState, AeMode, FlashState, AeBvTrigger) = (%d, %d, %d, %d, %d, %d)", __FUNCTION__, i4Index, m_rResultPool[i4Index].rEarlyCB.u1AfState, m_rResultPool[i4Index].rEarlyCB.u1AeState,
                m_rResultPool[i4Index].rEarlyCB.u1AeMode, m_rResultPool[i4Index].rEarlyCB.u1FlashState, m_rResultPool[i4Index].rEarlyCB.fgAeBvTrigger);

    return &m_rResultPool[i4Index].rEarlyCB;
}

AllResult_T*
ResultPoolImp::
getAllResultCur(MINT32 i4SttMagic)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_GET_1) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s]+ SttMagic:%d", __FUNCTION__, i4SttMagic);

    //===== Find capture index of stt megic =====
    for(MUINT32 i4Index = 0; i4Index < m_u4Capacity; i4Index++)
    {
        if(m_rResultPool[i4Index].rResultCfg.i4StatisticMagic == i4SttMagic)
        {
            CAM_LOGD("[%s] (Index, ResultStt) = (%d, %d)", __FUNCTION__, i4Index, m_rResultPool[i4Index].rResultCfg.i4StatisticMagic);
            return getAllResult(m_rResultPool[i4Index].rResultCfg.i4ReqMagic);
        }
        else
            CAM_LOGD_IF(fgLog, "[%s] getAllResultCur fail (In-ResultStt, Local-ResultStt, Index) = (%d, %d, %d)", __FUNCTION__, i4SttMagic, m_rResultPool[i4Index].rResultCfg.i4StatisticMagic, i4Index);
    }

    CAM_LOGD_IF(fgLog, "[%s]-", __FUNCTION__);
    return NULL;
}

MVOID
ResultPoolImp::
postCmdToThread(ResultPoolCmd4Convert_T* pCmd)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_OTHER) ? MTRUE: MFALSE;
    CAM_LOGD_IF(fgLog, "[%s]+ MagicNumber:%d SubsampleCount:%d", __FUNCTION__, pCmd->i4MagicNumReq, pCmd->i4SubsampleCount);
    //===== Post thread to convert =====
    m_pThreadResultPool->postCmd(pCmd);
    CAM_LOGD_IF(fgLog, "[%s]-", __FUNCTION__);
}

AllResult_T*
ResultPoolImp::
getAllResultLastCur(MINT32 i4ReqMagic)
{
    CAM_LOGD("[%s]+ ", __FUNCTION__);

    //===== ReqMagic to Index =====
    MINT32 i4Index = magic2IndexRingBuffer(i4ReqMagic);

    if(i4Index == -1)
    {
        CAM_LOGW("[%s]- Index is illegal value(%d)", __FUNCTION__, i4Index);
        return NULL;
    }

    //===== ResultPool ReqMagic need to equal input ReqMagic, if doesn't equal return NULL =====
    if(m_rResultPool[i4Index].rResultCfg.i4ReqMagic != i4ReqMagic)
    {
        CAM_LOGW("[%s]- ResultPool ReqMagic doesn't mapping input ReqMagic (%d, %d, %d)", __FUNCTION__, m_rResultPool[i4Index].rResultCfg.i4ReqMagic, i4ReqMagic, i4Index);
        return NULL;
    }

    CAM_LOGD("[%s]- (m_rResultPool[%d] Addr, ReqMagic) = (%p, %d)", __FUNCTION__, i4Index, &m_rResultPool[i4Index], i4ReqMagic);
    return &m_rResultPool[i4Index];
}

MVOID
ResultPoolImp::
updateLastInfo(LastInfo_T& LastInfo)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_LASTINFO) ? MTRUE: MFALSE;
    CAM_LOGD_IF(fgLog, "[%s] start lock", __FUNCTION__);
    Mutex::Autolock lock(m_LockLastInfo);
    CAM_LOGD_IF(fgLog, "[%s] source LastInfo copied(%d) +", __FUNCTION__, LastInfo.mBackupCamInfo_copied);
    ::memcpy(&m_vLastInfo, &LastInfo, sizeof(LastInfo));
    CAM_LOGD_IF(fgLog, "[%s] dest m_vLastInfo copied(%d) -", __FUNCTION__, m_vLastInfo.mBackupCamInfo_copied);
}

LastInfo_T&
ResultPoolImp::
getLastInfo()
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_LASTINFO) ? MTRUE: MFALSE;
    CAM_LOGD_IF(fgLog, "[%s] m_vLastInfo addr(%p) copied(%d)", __FUNCTION__, &m_vLastInfo, m_vLastInfo.mBackupCamInfo_copied);
    return m_vLastInfo;
}

MBOOL
ResultPoolImp::
lockLastInfo()
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_LASTINFO) ? MTRUE: MFALSE;
    CAM_LOGD_IF(fgLog, "[%s] +", __FUNCTION__);
    m_LockLastInfo.lock();
    CAM_LOGD_IF(fgLog, "[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
ResultPoolImp::
unlockLastInfo()
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_LASTINFO) ? MTRUE: MFALSE;
    CAM_LOGD_IF(fgLog, "[%s] +", __FUNCTION__);
    m_LockLastInfo.unlock();
    CAM_LOGD_IF(fgLog, "[%s] -", __FUNCTION__);
    return MTRUE;
}



#if (!CAM3_3A_ISP_30_EN)
MBOOL
ResultPoolImp::
getAAOInfo(MUINT32 u4GridPointNum, MINT32 i4SttMagic, AAO_PROC_INFO_T& rPSOInfo)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_AAOINFO) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s] SttMagic:%d, SubsampleCount:%d", __FUNCTION__, i4SttMagic, m_i4SubsampleCount);

    ::memset(&rPSOInfo, 0, sizeof(AAO_PROC_INFO_T));

    AllResult_T *pPreAllResult = getAllResult(i4SttMagic);
    if(pPreAllResult == NULL)
    {
        CAM_LOGD_IF(fgLog, "[%s] PreAllResult is NULL", __FUNCTION__);
        return MFALSE;
    }

    ISPResultToMeta_T *pISPResult = (ISPResultToMeta_T*)(pPreAllResult->ModuleResultAddr[E_ISP_RESULTTOMETA]->read());
    HALResultToMeta_T *pHALResult = (HALResultToMeta_T*)(pPreAllResult->ModuleResultAddr[E_HAL_RESULTTOMETA]->read());
    AEResultToMeta_T  *pAEResult = (AEResultToMeta_T*)(pPreAllResult->ModuleResultAddr[E_AE_RESULTTOMETA]->read());
    AWBResultToMeta_T *pAWBResult = (AWBResultToMeta_T*)(pPreAllResult->ModuleResultAddr[E_AWB_RESULTTOMETA]->read());

    CAM_LOGD_IF(fgLog, "[%s] (SttMagic, i4FrmId) = (%d, %d)", __FUNCTION__, i4SttMagic, pHALResult->i4FrmId);

    //===== For SMVR : 17 18 19 20,21 22 23 24 => if ReqMagic is 24 SttMagic is 20. But result only update 17, so need to get 17 address to get AAO info. =====
    if(m_i4SubsampleCount > 1)
    {
        if(pHALResult->i4FrmId != i4SttMagic && pHALResult->i4FrmId == 0)
        {
            MINT32 isUpdateMagic = pPreAllResult->rOld3AInfo.i4ConvertMagic[0];
            pPreAllResult = getAllResult(isUpdateMagic);
            if(pPreAllResult == NULL)
            {
                CAM_LOGW("[%s] pPreAllResult is NULL", __FUNCTION__);
                return MFALSE;
            }
            pISPResult = (ISPResultToMeta_T*)(pPreAllResult->ModuleResultAddr[E_ISP_RESULTTOMETA]->read());
            pHALResult = (HALResultToMeta_T*)(pPreAllResult->ModuleResultAddr[E_HAL_RESULTTOMETA]->read());
            pAEResult = (AEResultToMeta_T*)(pPreAllResult->ModuleResultAddr[E_AE_RESULTTOMETA]->read());
            pAWBResult = (AWBResultToMeta_T*)(pPreAllResult->ModuleResultAddr[E_AWB_RESULTTOMETA]->read());
            CAM_LOGD_IF(fgLog, "[%s] isUpdateMagic(%d) pHALResult->i4FrmId:%d", __FUNCTION__, isUpdateMagic, pHALResult->i4FrmId);
        }
    }

    if(pHALResult->i4FrmId != i4SttMagic && m_i4SubsampleCount <= 1)
    {
        CAM_LOGW("[%s] Get Result fail", __FUNCTION__);
        ::memset(&rPSOInfo.OB_Offset, 0, sizeof(rPSOInfo.OB_Offset));
        ::memset(&rPSOInfo.OB_Gain, 0, sizeof(rPSOInfo.OB_Gain));
        ::memset(&rPSOInfo.awb_gain, 0, sizeof(rPSOInfo.awb_gain));
        ::memset(&rPSOInfo.PreGain1, 0, sizeof(rPSOInfo.PreGain1));
        ::memset(&rPSOInfo.CCM, 0, sizeof(rPSOInfo.CCM));
        ::memset(&rPSOInfo.LSC_Gain_Grid_R, 0, sizeof(rPSOInfo.LSC_Gain_Grid_R));
        ::memset(&rPSOInfo.LSC_Gain_Grid_G, 0, sizeof(rPSOInfo.LSC_Gain_Grid_G));
        ::memset(&rPSOInfo.LSC_Gain_Grid_B, 0, sizeof(rPSOInfo.LSC_Gain_Grid_B));
        return MFALSE;
    }
    else
    {
        // OB offset
        rPSOInfo.OB_Offset[0] = pISPResult->rCamInfo.rOBC1.offst0.val;
        rPSOInfo.OB_Offset[1] = pISPResult->rCamInfo.rOBC1.offst1.val;
        rPSOInfo.OB_Offset[2] = pISPResult->rCamInfo.rOBC1.offst2.val;
        rPSOInfo.OB_Offset[3] = pISPResult->rCamInfo.rOBC1.offst3.val;
        // OB gain
        rPSOInfo.OB_Gain[0] = pISPResult->rCamInfo.rOBC1.gain0.val;
        rPSOInfo.OB_Gain[1] = pISPResult->rCamInfo.rOBC1.gain1.val;
        rPSOInfo.OB_Gain[2] = pISPResult->rCamInfo.rOBC1.gain2.val;
        rPSOInfo.OB_Gain[3] = pISPResult->rCamInfo.rOBC1.gain3.val;

        // AWB preGain1
        rPSOInfo.PreGain1 = pISPResult->rCamInfo.rAWBInfo.rPregain1;
        // AWB awbGain
        rPSOInfo.awb_gain = pISPResult->rCamInfo.rAWBInfo.rCurrentAWBGain;
        // CCM
        //memset(&rPSOInfo.CCM, 0, sizeof(rPSOInfo.CCM));
        ::memcpy(&rPSOInfo.CCM, &pISPResult->rCamInfo.rAWBInfo.rCscCCM, sizeof(rPSOInfo.CCM));

#if 0 //ISP5.0 not to use shading table
        // LSC
        std::vector<MUINT16*> myvec;

        myvec.push_back(&rPSOInfo.LSC_Gain_Grid_R[0]);
        myvec.push_back(&rPSOInfo.LSC_Gain_Grid_G[0]);
        myvec.push_back(&rPSOInfo.LSC_Gain_Grid_B[0]);

        // LSC
        ILscMgr* pLsc = ILscMgr::getInstance(static_cast<MUINT32>(m_rBasicCfg.u4SensorDev));
        MBOOL ret = pLsc->getGainTable(u4GridPointNum, i4SttMagic, myvec);

/*
        int k;
        for(k=0;k<10;k++) // Gain table defined in lsc_mgr always starts with B pixel.
        {
            CAM_LOGE("tbl3 k= %d, (%d, %d, %d)", k, rPSOInfo.LSC_Gain_Grid_R[k], rPSOInfo.LSC_Gain_Grid_G[k], rPSOInfo.LSC_Gain_Grid_B[k]);
        }
*/
        if(ret == MFALSE)
        {
            ::memset(&rPSOInfo.LSC_Gain_Grid_R, 0, sizeof(rPSOInfo.LSC_Gain_Grid_R));
            ::memset(&rPSOInfo.LSC_Gain_Grid_G, 0, sizeof(rPSOInfo.LSC_Gain_Grid_G));
            ::memset(&rPSOInfo.LSC_Gain_Grid_B, 0, sizeof(rPSOInfo.LSC_Gain_Grid_B));
        }
#endif
    }
    return MTRUE;
}

MBOOL
ResultPoolImp::
getAAOInfoTest(MUINT32 u4GridPointNum, MINT32 i4SttMagic, AAO_PROC_INFO_T& rPSOInfo)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_AAOINFO) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s] SttMagic:%d", __FUNCTION__, i4SttMagic);

    AllResult_T *pPreAllResult = getAllResult(i4SttMagic);
    if(pPreAllResult == NULL)
    {
        CAM_LOGW("[%s] PreAllResult is NULL", __FUNCTION__);
        return MFALSE;
    }

    ISPResultToMeta_T *pISPResult = (ISPResultToMeta_T*)(pPreAllResult->ModuleResultAddr[E_ISP_RESULTTOMETA]->read());
    HALResultToMeta_T *pHALResult = (HALResultToMeta_T*)(pPreAllResult->ModuleResultAddr[E_HAL_RESULTTOMETA]->read());
    AEResultToMeta_T  *pAEResult = (AEResultToMeta_T*)(pPreAllResult->ModuleResultAddr[E_AE_RESULTTOMETA]->read());
    AWBResultToMeta_T *pAWBResult = (AWBResultToMeta_T*)(pPreAllResult->ModuleResultAddr[E_AWB_RESULTTOMETA]->read());

    CAM_LOGD_IF(fgLog, "[%s] (SttMagic, i4FrmId) = (%d, %d)", __FUNCTION__, i4SttMagic, pHALResult->i4FrmId);

    //===== For SMVR : 17 18 19 20,21 22 23 24 => if ReqMagic is 24 SttMagic is 20. But result only update 17, so need to get 17 address to get AAO info. =====
    if(m_i4SubsampleCount > 1)
    {
        if(pHALResult->i4FrmId != i4SttMagic && pHALResult->i4FrmId == 0)
        {
            MINT32 isUpdateMagic = pPreAllResult->rOld3AInfo.i4ConvertMagic[0];
            pPreAllResult = getAllResult(isUpdateMagic);
            if(pPreAllResult == NULL)
            {
                CAM_LOGW("[%s] pPreAllResult is NULL", __FUNCTION__);
                return MFALSE;
            }
            pISPResult = (ISPResultToMeta_T*)(pPreAllResult->ModuleResultAddr[E_ISP_RESULTTOMETA]->read());
            pHALResult = (HALResultToMeta_T*)(pPreAllResult->ModuleResultAddr[E_HAL_RESULTTOMETA]->read());
            pAEResult = (AEResultToMeta_T*)(pPreAllResult->ModuleResultAddr[E_AE_RESULTTOMETA]->read());
            pAWBResult = (AWBResultToMeta_T*)(pPreAllResult->ModuleResultAddr[E_AWB_RESULTTOMETA]->read());
            CAM_LOGD_IF(fgLog, "[%s] isUpdateMagic(%d) pHALResult->i4FrmId:%d", __FUNCTION__, isUpdateMagic, pHALResult->i4FrmId);
        }
    }

    if(pHALResult->i4FrmId != i4SttMagic && m_i4SubsampleCount <= 1)
    {
        CAM_LOGW("[%s] Get Result fail", __FUNCTION__);
        ::memset(&rPSOInfo.OB_Offset, 0, sizeof(rPSOInfo.OB_Offset));
        ::memset(&rPSOInfo.OB_Gain, 0, sizeof(rPSOInfo.OB_Gain));
        ::memset(&rPSOInfo.awb_gain, 0, sizeof(rPSOInfo.awb_gain));
        ::memset(&rPSOInfo.PreGain1, 0, sizeof(rPSOInfo.PreGain1));
        ::memset(&rPSOInfo.CCM, 0, sizeof(rPSOInfo.CCM));
        ::memset(&rPSOInfo.LSC_Gain_Grid_R, 0, sizeof(rPSOInfo.LSC_Gain_Grid_R));
        ::memset(&rPSOInfo.LSC_Gain_Grid_G, 0, sizeof(rPSOInfo.LSC_Gain_Grid_G));
        ::memset(&rPSOInfo.LSC_Gain_Grid_B, 0, sizeof(rPSOInfo.LSC_Gain_Grid_B));
        return MFALSE;
    }
    else
    {
        // OB offset
        rPSOInfo.OB_Offset[0] = pISPResult->rCamInfo.rOBC1.offst0.val;
        rPSOInfo.OB_Offset[1] = pISPResult->rCamInfo.rOBC1.offst1.val;
        rPSOInfo.OB_Offset[2] = pISPResult->rCamInfo.rOBC1.offst2.val;
        rPSOInfo.OB_Offset[3] = pISPResult->rCamInfo.rOBC1.offst3.val;
        // OB gain
        rPSOInfo.OB_Gain[0] = pISPResult->rCamInfo.rOBC1.gain0.val;
        rPSOInfo.OB_Gain[1] = pISPResult->rCamInfo.rOBC1.gain1.val;
        rPSOInfo.OB_Gain[2] = pISPResult->rCamInfo.rOBC1.gain2.val;
        rPSOInfo.OB_Gain[3] = pISPResult->rCamInfo.rOBC1.gain3.val;

        // AWB preGain1
        rPSOInfo.PreGain1 = pISPResult->rCamInfo.rAWBInfo.rPregain1;
        // AWB awbGain
        rPSOInfo.awb_gain = pISPResult->rCamInfo.rAWBInfo.rCurrentAWBGain;
        // CCM
        //memset(&rPSOInfo.CCM, 0, sizeof(rPSOInfo.CCM));
        ::memcpy(&rPSOInfo.CCM, &pISPResult->rCamInfo.rAWBInfo.rCscCCM, sizeof(rPSOInfo.CCM));

        // LSC
        ILscTbl rLscTbl(ILscTable::GAIN_FIXED);
        ILscMgr* pLsc = ILscMgr::getInstance(static_cast<MUINT32>(m_rBasicCfg.u4SensorDev));
        MBOOL ret = pLsc->getGainTable(u4GridPointNum, i4SttMagic, rLscTbl);

        if(ret && rLscTbl.getSize() != 0)
        {
            const MUINT16* rLSCGain = static_cast<const MUINT16*>(rLscTbl.getData());
            for(MUINT32 i=0; i<u4GridPointNum*u4GridPointNum; i++) // Gain table defined in lsc_mgr always starts with B pixel.
            {
                rPSOInfo.LSC_Gain_Grid_B[i] = *(rLSCGain++);
                rPSOInfo.LSC_Gain_Grid_G[i] = *(rLSCGain++);
                rLSCGain++;
                rPSOInfo.LSC_Gain_Grid_R[i] = *(rLSCGain++);
            }
        }
        else
        {
            ::memset(&rPSOInfo.LSC_Gain_Grid_R, 0, sizeof(rPSOInfo.LSC_Gain_Grid_R));
            ::memset(&rPSOInfo.LSC_Gain_Grid_G, 0, sizeof(rPSOInfo.LSC_Gain_Grid_G));
            ::memset(&rPSOInfo.LSC_Gain_Grid_B, 0, sizeof(rPSOInfo.LSC_Gain_Grid_B));
        }

        //CAM_LOGE("[%s] NoN-OP(%d %d %d)", __FUNCTION__, rPSOInfo.LSC_Gain_Grid_B[5], rPSOInfo.LSC_Gain_Grid_G[5], rPSOInfo.LSC_Gain_Grid_R[5]);

    }
    return MTRUE;
}

#endif
