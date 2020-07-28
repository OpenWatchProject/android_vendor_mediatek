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

#include "IResultPool.h"
#include "ResultPoolImpWrapper.h"
#include <ThreadResultPool.h>
#include <mtkcam/utils/std/Log.h>

// standard libraty
#include <list>
#include <vector>

// utils header
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/hw/HwTransform.h>

// 3A private header
#include <private/aaa_hal_private.h>

#include <mtkcam/utils/exif/IBaseCamExif.h>

// 3A Module header
#include <mtkcam/aaa/IDngInfo.h>
#if CAM3_LSC_FEATURE_EN
#include <lsc/ILscMgr.h>
#endif

// Systrace header
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>

#include <ResultPool4Module.h>

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
#define HAL3RESULTPOOL_LOG_LOCK     (1<<10)

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
convert3AROI(const std::vector<MINT32> &vec3AROI, AllMetaResult_T* pMetaResult, const mtk_camera_metadata_tag_t &tag) const
{
    MBOOL fgLogArea = (ResultPoolImp::m_u4LogEn & HAL3RESULTPOOL_LOG_AREA) ? MTRUE: MFALSE;

    // order is Type,Number of ROI,left,top,right,bottom,Result, left,top,right,bottom,Result...
    CAM_LOGD_IF(fgLogArea, "[%s] size(%d) pMetaResult:%p", __FUNCTION__, vec3AROI.size(), pMetaResult);
    if( vec3AROI.size()!=0)
    {
        IMetadata::IEntry entryNew(tag);
        const MINT32 *ptrROIInfo = &(vec3AROI[0]);
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
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(ModuleResult4AF);
    const HALResultToMeta_T*   pHALResult = ( (HALResultToMeta_T*)(pAllResult->ModuleResult[E_HAL_RESULTTOMETA]->read()) );
    const AFResultToMeta_T*    pAFResult = ( (AFResultToMeta_T*)(pAllResult->ModuleResult[E_AF_RESULTTOMETA]->read()) );
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(queryAFInfoToCompare);
    // query appMeta info to compare
    MUINT8 u1AfTrig = 0;
    QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_CONTROL_AF_TRIGGER, u1AfTrig);

#if 0 //Debug info whith non-perframe
    if(m_r3AImportantInfo.u1AfState != pAFResult->u1AfState || m_r3AImportantInfo.u1AfTrig != u1AfTrig)
    {
        CAM_LOGD("[%s] Req (#%d) AfState(%d->%d) AfTrig(%d->%d)",
            __FUNCTION__, pMetaResult->rMetaResult.MagicNum, m_r3AImportantInfo.u1AfState, pAFResult->u1AfState, m_r3AImportantInfo.u1AfTrig, u1AfTrig);
    }
#endif
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(convertAFInfoToMeta);
    // convert result into metadata
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_CONTROL_AF_STATE, pAFResult->u1AfState);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_LENS_STATE, pAFResult->u1LensState);
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(convertLensToMeta);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_LENS_FOCUS_DISTANCE, pAFResult->fLensFocusDistance);
    UPDATE_ENTRY_ARRAY(pMetaResult->rMetaResult.appMeta, MTK_LENS_FOCUS_RANGE, pAFResult->fLensFocusRange, 2);
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(convertAFExifToMeta);
    MINT32 i4VecExifSize = 0;
    MINT32 i4VecDbg3ASize = 0;
    // Exif
    {
        i4VecExifSize = pAllResult->vecExifInfo.size();
        i4VecDbg3ASize = pAllResult->vecDbg3AInfo.size();
        if (i4VecExifSize)
        {
            IMetadata metaExif;
            QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_3A_EXIF_METADATA, metaExif);
            //UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_FNUMBER,              fFNum*10/*rExifInfo.u4FNumber*/);
            //UPDATE_ENTRY_SINGLE<MINT32>(metaExif, MTK_3A_EXIF_FOCAL_LENGTH,         fFocusLength*1000/*rExifInfo.u4FocalLength*/);

            // debug info
            if (i4VecDbg3ASize)
            {
                IMetadata::Memory dbg3A;
                //AAA_DEBUG_INFO2_T& rDbgIspInfo = *reinterpret_cast<AAA_DEBUG_INFO2_T*>(pAllResult->vecDbgIspInfo.editArray());
                //IAfMgr::getInstance().getDebugInfo(m_rBasicCfg.u4SensorDev, rDbgIspInfo.rAFDebugInfo);
                dbg3A.appendVector(pAllResult->vecDbg3AInfo);
                UPDATE_ENTRY_SINGLE(metaExif, MTK_3A_EXIF_DBGINFO_AAA_DATA, dbg3A);
            }
            UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_3A_EXIF_METADATA, metaExif);
        }
    }
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(printAFRegion);
    // log of control
    MINT32 rRgn[5] = {};
    MUINT8 u1Mode;
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
        const MINT32 *ptrPos = &(pAllResult->vecFocusAreaPos[0]);
        MINT32  cvtX = 0, cvtY = 0, cvtW = 0, cvtH = 0;
        std::vector<MINT32> vecCvtPos = pAllResult->vecFocusAreaPos;
        MINT32 *ptrCvtPos = &(vecCvtPos[0]);
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
        UPDATE_ENTRY_ARRAY(pMetaResult->rMetaResult.halMeta, MTK_FOCUS_AREA_POSITION, &vecCvtPos[0], vecCvtPos.size());
        UPDATE_ENTRY_ARRAY(pMetaResult->rMetaResult.halMeta, MTK_FOCUS_AREA_RESULT, &(pAllResult->vecFocusAreaRes[0]), pAllResult->vecFocusAreaRes.size());
    }
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(convertAFROI);
    CAM_LOGD_IF(fgLogArea, "[%s] MTK_3A_FEATURE_AF_ROI", __FUNCTION__);
    convert3AROI(pAllResult->vecAFROI, pMetaResult, MTK_3A_FEATURE_AF_ROI);
    AAA_TRACE_END_HAL;

#if 0 //Debug info whith non-perframe
    AAA_TRACE_HAL(updateImportantInfo4AF);
    // update Important Info as pre-info
    m_r3AImportantInfo.u1AfState = pAFResult->u1AfState;
    m_r3AImportantInfo.u1AfTrig = u1AfTrig;
    AAA_TRACE_END_HAL;
#endif

    pMetaResult->i4IsConverted |= E_AF_CONVERT;

    CAM_LOGD("[%s] Done Req(#%d) (pResult,pMeta)(%p,%p) i4IsConverted(%d) AfState(%d) LensState(%d) AfTrig(%d) FocusDist(%f) FocusRange(%f,%f) (VecExifSize,VecDbg3ASize)(%d,%d)",
            __FUNCTION__, pMetaResult->rMetaResult.MagicNum, pAllResult, pMetaResult, pMetaResult->i4IsConverted, pAFResult->u1AfState, pAFResult->u1LensState, u1AfTrig
                        , pAFResult->fLensFocusDistance, pAFResult->fLensFocusRange[0], pAFResult->fLensFocusRange[1]
                        , i4VecExifSize, i4VecDbg3ASize);
    AAA_TRACE_END_HAL;
    return MFALSE;

}

MINT32
ResultPoolImp::
convertToMetadata42A(AllResult_T* pAllResult, AllMetaResult_T* pMetaResult)
{
    AAA_TRACE_HAL(assigneValue42A);
    MBOOL fgLogCVT = (m_u4LogEn & HAL3RESULTPOOL_LOG_CVT_0) ? MTRUE: MFALSE;
    MBOOL fgLogArea = (m_u4LogEn & HAL3RESULTPOOL_LOG_AREA) ? MTRUE: MFALSE;
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(ModuleResult42A);
    const HALResultToMeta_T*   pHALResult = ( (HALResultToMeta_T*)(pAllResult->ModuleResult[E_HAL_RESULTTOMETA]->read()) );
    const AEResultToMeta_T*    pAEResult = ( (AEResultToMeta_T*)(pAllResult->ModuleResult[E_AE_RESULTTOMETA]->read()) );
    const AWBResultToMeta_T*   pAWBResult = ( (AWBResultToMeta_T*)(pAllResult->ModuleResult[E_AWB_RESULTTOMETA]->read()) );
    const ISPResultToMeta_T*   pISPResult = ( (ISPResultToMeta_T*)(pAllResult->ModuleResult[E_ISP_RESULTTOMETA]->read()) );
    const LSCResultToMeta_T*   pLSCResult = ( (LSCResultToMeta_T*)(pAllResult->ModuleResult[E_LSC_RESULTTOMETA]->read()) );
    const FLASHResultToMeta_T* pFLASHResult = ( (FLASHResultToMeta_T*)(pAllResult->ModuleResult[E_FLASH_RESULTTOMETA]->read()) );
    const FLKResultToMeta_T*   pFLKResult = ( (FLKResultToMeta_T*)(pAllResult->ModuleResult[E_FLK_RESULTTOMETA]->read()) );
    const AEResultInfo_T  *    pAEResultInfo = (AEResultInfo_T*)(pAllResult->ModuleResult[E_AE_RESULTINFO]->read());
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

#if 0 //Debug info whith non-perframe
    AAA_TRACE_HAL(query2AInfoToCompare);
    // query appMeta info to compare
    MUINT8 u1CapIntent;
    MUINT8 u1AePrecapTrig;
    QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_CONTROL_CAPTURE_INTENT, u1CapIntent);
    QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_CONTROL_AE_PRECAPTURE_TRIGGER, u1AePrecapTrig);

    if(m_r3AImportantInfo.u1AeState != pAEResult->u1AeState || m_r3AImportantInfo.u1AwbState != pAWBResult->u1AwbState ||
        m_r3AImportantInfo.u1FlashState != pFLASHResult->u1FlashState || m_r3AImportantInfo.i8SensorExposureTime != pAEResult->i8SensorExposureTime ||
        m_r3AImportantInfo.i8SensorFrameDuration != pAEResult->i8SensorFrameDuration || m_r3AImportantInfo.i4SensorSensitivity != pAEResult->i4SensorSensitivity ||
        m_r3AImportantInfo.u1CapIntent != u1CapIntent || m_r3AImportantInfo.u1AePrecapTrig != u1AePrecapTrig)
    {
        CAM_LOGD("[%s] Req(#%d) AeState(%d->%d) AwbState(%d->%d) FlashState(%d->%d) ExpTime(%lld->%lld) FrmDuration(%lld->%lld) ISO(%d->%d) CapIntent(%d->%d) AePrecapTrig(%d->%d) (fFNum, fFocusLength)(%.5f, %.5f)",
            __FUNCTION__, pHALResult->i4FrmId, m_r3AImportantInfo.u1AeState, pAEResult->u1AeState, m_r3AImportantInfo.u1AwbState, pAWBResult->u1AwbState,
            m_r3AImportantInfo.u1FlashState, pFLASHResult->u1FlashState, m_r3AImportantInfo.i8SensorExposureTime, pAEResult->i8SensorExposureTime,
            m_r3AImportantInfo.i8SensorFrameDuration, pAEResult->i8SensorFrameDuration, m_r3AImportantInfo.i4SensorSensitivity, pAEResult->i4SensorSensitivity,
            m_r3AImportantInfo.u1CapIntent, u1CapIntent, m_r3AImportantInfo.u1AePrecapTrig, u1AePrecapTrig, fFNum, fFocusLength);
    }
    AAA_TRACE_END_HAL;
#endif

    AAA_TRACE_HAL(convert2AInfoToMeta);
    // convert result into metadata
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_CONTROL_SCENE_MODE, pHALResult->u1SceneMode);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_CONTROL_AWB_STATE, pAWBResult->u1AwbState);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_CONTROL_AE_STATE, pAEResult->u1AeState);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_FLASH_STATE, pFLASHResult->u1FlashState);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_STATISTICS_SCENE_FLICKER, pFLKResult->u1SceneFlk);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_HDR_FEATURE_HDR_DETECTION_RESULT, pAEResult->i4AutoHdrResult);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_3A_AE_BV_TRIGGER, pAEResult->fgAeBvTrigger);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_3A_AE_LV_VALUE, pAEResultInfo->AEPerframeInfo.rAEISPInfo.i4LightValue_x10);
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

    AAA_TRACE_HAL(convertSensorInfoToMeta);
    // sensor
    IDngInfo* pDngInfo = nullptr;
    IMetadata rMetaDngDynNoiseProfile = {};
    pDngInfo = MAKE_DngInfo(LOG_TAG, m_rBasicCfg.i4SensorIdx);
    if (pDngInfo)
    {
        rMetaDngDynNoiseProfile = pDngInfo->getDynamicNoiseProfile(pAEResult->i4SensorSensitivity);
    }
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_SENSOR_EXPOSURE_TIME, pAEResult->i8SensorExposureTime);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_SENSOR_FRAME_DURATION, pAEResult->i8SensorFrameDuration);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_SENSOR_SENSITIVITY, pAEResult->i4SensorSensitivity);
    MUINT8 u1RollingShutterSkew = GET_ENTRY_SINGLE_IN_ARRAY(m_metaStaticInfo, MTK_REQUEST_AVAILABLE_RESULT_KEYS, (MINT32)MTK_SENSOR_ROLLING_SHUTTER_SKEW);

    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_SENSOR_GREEN_SPLIT, (MFLOAT) 0.0f);
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
    UPDATE_ENTRY_ARRAY(pMetaResult->rMetaResult.appMeta, MTK_SENSOR_NEUTRAL_COLOR_POINT, NeutralColorPt, 3);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_COLOR_CORRECTION_ABERRATION_MODE, (MUINT8) MTK_COLOR_CORRECTION_ABERRATION_MODE_OFF);
    UPDATE_ENTRY_ARRAY(pMetaResult->rMetaResult.appMeta, MTK_COLOR_CORRECTION_GAINS, pAWBResult->fColorCorrectGain, 4);
    UPDATE_ENTRY_ARRAY(pMetaResult->rMetaResult.appMeta, MTK_3A_FEATURE_AWB_AVAILABL_RANGE, pAWBResult->i4AwbAvailableRange, 2);
    CAM_LOGD_IF(fgLogCVT, "[%s] Awb Gain(%3.6f, %3.6f, %3.6f, %3.6f), Awb Available Range(%d, %d)", __FUNCTION__,
            pAWBResult->fColorCorrectGain[0], pAWBResult->fColorCorrectGain[1], pAWBResult->fColorCorrectGain[2], pAWBResult->fColorCorrectGain[3],
            pAWBResult->i4AwbAvailableRange[0], pAWBResult->i4AwbAvailableRange[1]);

    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_3A_MANUAL_AWB_COLORTEMPERATURE_MAX, pAWBResult->i4MWBColorTemperatureMax);
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_3A_MANUAL_AWB_COLORTEMPERATURE_MIN, pAWBResult->i4MWBColorTemperatureMin);
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(convertLSCInfoToMeta);
    IMetadata::Memory rLscData;
    IMetadata::Memory rTsfData;
    IMetadata::Memory rTsfDumpNo;
    // shading
    MINT32 i4LscDataSize = 0;
    if(pLSCResult->i4CurrTblIndex != -1)
    {
        NSIspTuning::ILscMgr* pLsc = NSIspTuning::ILscMgr::getInstance(static_cast<MUINT32>(m_rBasicCfg.u4SensorDev));
        std::vector<MUINT8> d, e, f;
        pLsc->getCurrTbl((MUINT32)(pLSCResult->i4CurrTblIndex), d);

        // for appendVector(android::Vector &v);
        android::Vector<MUINT8> d_temp;
        d_temp.resize( d.size() );
        ::memcpy( static_cast<MVOID*>(d_temp.editArray()), d.data(), d.size() );
        //
        //rLscData.appendVector(d);
        rLscData.appendVector(d_temp);

        pLsc->getCorresTsfo((MUINT32)(pLSCResult->i4CurrTsfIndex), e);

        // for appendVector(android::Vector &v);
        android::Vector<MUINT8> e_temp;
        e_temp.resize( e.size() );
        ::memcpy( static_cast<MVOID*>(e_temp.editArray()), e.data(), e.size() );
        //
        //rTsfData.appendVector(e);
        rTsfData.appendVector(e_temp);

        char namingPostfix[32];
        pLsc->CCTOPGetSdblkFileCfg(namingPostfix);

        f.resize(32);

        ::memcpy(f.data(), namingPostfix, 32);

        // for appendVector(android::Vector &v);
        android::Vector<MUINT8> f_temp;
        f_temp.resize( f.size() );
        ::memcpy( static_cast<MVOID*>(f_temp.editArray()), f.data(), f.size() );
        //
        //rTsfData.appendVector(f);
        rTsfDumpNo.appendVector(f_temp);

        i4LscDataSize = rLscData.size();
        UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_LSC_TBL_DATA, rLscData);
        UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_LSC_TSF_DATA, rTsfData);
        UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_LSC_TSF_DUMP_NO, rTsfDumpNo);
    }

    if (pLSCResult->fgShadingMapOn)
    {
        MUINT8 u1ShadingMode = 0;
        QUERY_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_SHADING_MODE, u1ShadingMode);
        if (pDngInfo)
        {
            pMetaResult->rMetaResult.appMeta += pDngInfo->getShadingMapFromMem(u1ShadingMode, rLscData);//ToDo other file
        }
    }
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(convertTuningInfoToMeta);

    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_REAL_LV, pISPResult->rCamInfo.rAEInfo.i4RealLightValue_x10);

    //UPDATE_ENTRY_ARRAY(rMetaResult.halMeta, MTK_PROCESSOR_CAMINFO, rResult.rCamInfo.data, sizeof(NSIspTuning::RAWIspCamInfo_U));
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(convert2AExifToMeta);
    MINT32 i4VecExifSize = 0;
    MINT32 i4VecDbg3ASize = 0;
    MINT32 i4VecDbgShadSize = 0;
    MINT32 i4VecDbgIspSize = 0;
    // Exif
    {
        i4VecExifSize = pAllResult->vecExifInfo.size();
        i4VecDbg3ASize = pAllResult->vecDbg3AInfo.size();
        i4VecDbgShadSize = pAllResult->vecDbgShadTbl.size();
        i4VecDbgIspSize = pAllResult->vecDbgIspInfo.size();
        if (i4VecExifSize)
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

            // debug info
            if (i4VecDbg3ASize && i4VecDbgShadSize && i4VecDbgIspSize)
            {
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

            UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.halMeta, MTK_ANALOG_GAIN, (MINT32)(MINT32)pAEResultInfo->AEPerframeInfo.rAEISPInfo.u4P1SensorGain);
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

    AAA_TRACE_HAL(convert2AROI);
    CAM_LOGD_IF(fgLogArea, "[%s] MTK_3A_FEATURE_AE_ROI", __FUNCTION__);
    convert3AROI(pAllResult->vecAEROI, pMetaResult, MTK_3A_FEATURE_AE_ROI);

    CAM_LOGD_IF(fgLogArea, "[%s] MTK_3A_FEATURE_AWB_ROI", __FUNCTION__);
    convert3AROI(pAllResult->vecAWBROI, pMetaResult, MTK_3A_FEATURE_AWB_ROI);
    AAA_TRACE_END_HAL;

    AAA_TRACE_HAL(convertStereoROI);
    // stereo warning message
    UPDATE_ENTRY_SINGLE(pMetaResult->rMetaResult.appMeta, MTK_STEREO_FEATURE_WARNING, pHALResult->i4StereoWarning);
    AAA_TRACE_END_HAL;

    HAL3APerframeInfo_T r3APerframeInfo;
    r3APerframeInfo.u4P1DGNGain = pAEResultInfo->AEPerframeInfo.rAEISPInfo.u4P1DGNGain;
    UPDATE_MEMORY(pMetaResult->rMetaResult.halMeta, MTK_3A_PERFRAME_INFO, r3APerframeInfo);

#if 0 //Debug info whith non-perframe
    AAA_TRACE_HAL(updateImportantInfo42A);
    // update Important Info as pre-info
    m_r3AImportantInfo.u1AeState = pAEResult->u1AeState;
    m_r3AImportantInfo.u1AwbState = pAWBResult->u1AwbState;
    m_r3AImportantInfo.u1FlashState = pFLASHResult->u1FlashState;
    m_r3AImportantInfo.i8SensorExposureTime = pAEResult->i8SensorExposureTime;
    m_r3AImportantInfo.i8SensorFrameDuration = pAEResult->i8SensorFrameDuration;
    m_r3AImportantInfo.i4SensorSensitivity = pAEResult->i4SensorSensitivity;
    m_r3AImportantInfo.u1CapIntent = u1CapIntent;
    m_r3AImportantInfo.u1AePrecapTrig = u1AePrecapTrig;
    AAA_TRACE_END_HAL;
#endif

    pMetaResult->i4IsConverted |= E_2A_CONVERT;

    CAM_LOGD("[%s] Done Req(#%d) (pResult,pMeta)(%p,%p) i4IsConverted(%d) (Ae,Awb,Flash State)(%d,%d,%d) RSS(%lld) ExpTime(%lld) FrmDuration(%lld) ISO(%d) CurTblIdx(%d) LscSize(%d) (VecExifSize,VecDbg3ASize,VecDbgShadSize,VecDbgIspSize)(%d,%d,%d,%d) 3APerframeInfo.DGNGain(%d)",
            __FUNCTION__, pHALResult->i4FrmId, pAllResult, pMetaResult, pMetaResult->i4IsConverted, pAEResult->u1AeState, pAWBResult->u1AwbState, pFLASHResult->u1FlashState, pAEResult->i8SensorRollingShutterSkew,
            pAEResult->i8SensorExposureTime, pAEResult->i8SensorFrameDuration, pAEResult->i4SensorSensitivity, pLSCResult->i4CurrTblIndex, i4LscDataSize, i4VecExifSize, i4VecDbg3ASize, i4VecDbgShadSize, i4VecDbgIspSize, r3APerframeInfo.u4P1DGNGain);

    return MFALSE;
}

MINT32
ResultPoolImp::
convertToMetadataFlow(const MINT32& i4ReqMagic, MINT32 i4ConvertType, AllResult_T* pAllResult, AllMetaResult_T* pMetaResult)
{
    /*****************************
     *     convert result to metadata and update metedata to MW
     *****************************/
    MBOOL fgLogCVT = (m_u4LogEn & HAL3RESULTPOOL_LOG_CVT_0) ? MTRUE: MFALSE;

    MINT32 i4IsConvert = (E_2A_CONVERT|E_AF_CONVERT);
    MINT32 i4IsConvert4MetaResult = E_NON_CONVERT;
        i4IsConvert4MetaResult = (pMetaResult->i4IsConverted);

    CAM_LOGD_IF(fgLogCVT, "[%s] Req(#%d) (pResult,pMeta)(%p,%p) i4IsConvert4MetaResult(%d)",
                __FUNCTION__, i4ReqMagic, pAllResult, pMetaResult, i4IsConvert4MetaResult);

    // Convert subsample request to metadatas
    if(m_i4SubsampleCount > 1)
        convertSubsampleResult4SMVR(i4ReqMagic, i4ConvertType, pAllResult, pMetaResult);


    //Normal flow
    if(m_i4SubsampleCount <= 1)
    {
        MINT32 i4Validate = (isValidateByReqNum(i4ReqMagic, E_HAL_RESULTTOMETA) && isValidateByReqNum(i4ReqMagic, E_AE_RESULTTOMETA) &&
                             /*isValidateByReqNum(i4ReqMagic, E_AF_RESULTTOMETA) &&*/ isValidateByReqNum(i4ReqMagic, E_AWB_RESULTTOMETA) &&
                             isValidateByReqNum(i4ReqMagic, E_LSC_RESULTTOMETA) && isValidateByReqNum(i4ReqMagic, E_FLASH_RESULTTOMETA) &&
                             isValidateByReqNum(i4ReqMagic, E_FLK_RESULTTOMETA) /*&& isValidateByReqNum(i4ReqMagic, E_ISP_RESULTTOMETA)*/);
        if(i4Validate == MTRUE && (i4ConvertType & E_2A_CONVERT))
        {
            convertToMetadata42A(pAllResult, pMetaResult);
        }
        if(isValidateByReqNum(i4ReqMagic, E_AF_RESULTTOMETA) && (i4ConvertType & E_AF_CONVERT))
            convertToMetadata4AF(pAllResult, pMetaResult);
    }

    i4IsConvert4MetaResult = (pMetaResult->i4IsConverted);
    CAM_LOGD("[%s] Done Req(#%d) (pResult,pMeta)(%p,%p) i4IsConverted(%d) SubsampleCount(%d)",
                __FUNCTION__, i4ReqMagic, pAllResult, pMetaResult, i4IsConvert4MetaResult, m_i4SubsampleCount);
    return MFALSE;
}

MINT32
ResultPoolImp::
convertSubsampleResult4SMVR(const MINT32& i4ReqMagic, MINT32 i4ConvertType, AllResult_T* pAllResult, AllMetaResult_T* pMetaResult)
{
    MINT32 i4RetReqMagic = 0;

    if(m_i4SubsampleCount > 1)
    {
        SLOW_MOTION_3A_INFO_T rLocalNew3AInfo;
        SLOW_MOTION_3A_INFO_T rLocalOld3AInfo;

        //===== Get new result =====
        if(pAllResult == NULL)
            CAM_LOGE("[%s] pAllResult NULL", __FUNCTION__);
        else
        {
            //===== update new to local =====
            AEResultToMeta_T *pAEResult = (AEResultToMeta_T*)(pAllResult->ModuleResult[E_AE_RESULTTOMETA]->read());
            AFResultToMeta_T *pAFResult = (AFResultToMeta_T*)(pAllResult->ModuleResult[E_AF_RESULTTOMETA]->read());
            rLocalNew3AInfo.u1AeState = pAEResult->u1AeState;
            rLocalNew3AInfo.i8SensorExposureTime = pAEResult->i8SensorExposureTime;
            rLocalNew3AInfo.i4SensorSensitivity = pAEResult->i4SensorSensitivity;
            rLocalNew3AInfo.rAfInfo.u1AfState = pAFResult->u1AfState;

            //===== old setting update to new setting =====
            rLocalOld3AInfo.u1AeState = pAllResult->rOld3AInfo.u1AeState;
            rLocalOld3AInfo.i8SensorExposureTime = pAllResult->rOld3AInfo.i8SensorExposureTime;
            rLocalOld3AInfo.i4SensorSensitivity = pAllResult->rOld3AInfo.i4SensorSensitivity;
            rLocalOld3AInfo.rAfInfo.u1AfState = pAllResult->rOld3AInfo.rAfInfo.u1AfState;

            pAEResult->u1AeState = rLocalOld3AInfo.u1AeState;
            pAEResult->i8SensorExposureTime = rLocalOld3AInfo.i8SensorExposureTime;
            pAEResult->i4SensorSensitivity = rLocalOld3AInfo.i4SensorSensitivity;
            pAFResult->u1AfState = rLocalOld3AInfo.rAfInfo.u1AfState;

            //===== One result to update 4 metadata =====
            //for(MINT32 i=0; i < m_i4SubsampleCount; i++)
            {
                //i4RetReqMagic = pAllResult->rOld3AInfo.i4ConvertMagic[i];
                if( pAFResult->u1AfState != rLocalNew3AInfo.rAfInfo.u1AfState)
                    pAFResult->u1AfState = rLocalNew3AInfo.rAfInfo.u1AfState;

                if( pAEResult->u1AeState != rLocalNew3AInfo.u1AeState)
                    pAEResult->u1AeState = rLocalNew3AInfo.u1AeState;

                if( (pAEResult->i8SensorExposureTime != rLocalNew3AInfo.i8SensorExposureTime) && (m_i4SubsampleCount >= pAllResult->rOld3AInfo.rGainDelay.u4SutterGain_Delay) )
                    pAEResult->i8SensorExposureTime = rLocalNew3AInfo.i8SensorExposureTime;

                if( (pAEResult->i4SensorSensitivity != rLocalNew3AInfo.i4SensorSensitivity) && (m_i4SubsampleCount >= pAllResult->rOld3AInfo.rGainDelay.u4SensorGain_Delay) )
                    pAEResult->i4SensorSensitivity = rLocalNew3AInfo.i4SensorSensitivity;

                CAM_LOGD("[%s] SMVR (ConvertType, pMetaResult, pAllResult) = (%d, %p, %p)", __FUNCTION__, i4ConvertType, pMetaResult, pAllResult);

                if(i4ConvertType & E_AF_CONVERT)
                {
                    if(pMetaResult != NULL)
                        convertToMetadata4AF(pAllResult, pMetaResult);
                    else
                        MY_LOGE("[%s] pMetaResult NULL(%p)", __FUNCTION__, pMetaResult);
                }

                if(i4ConvertType & E_2A_CONVERT)
                {
                    if(pMetaResult != NULL)
                        convertToMetadata42A(pAllResult, pMetaResult);
                else
                    MY_LOGE("[%s] pMetaResult NULL(%p)", __FUNCTION__, pMetaResult);
                }
            }
        }
    }

    return i4RetReqMagic;
}

IResultPool*
IResultPool::
getInstance(MINT32 i4SensorDevId)
{
    return ResultPoolImpWrapper::getInstance(i4SensorDevId);
}

IResultPool*
ResultPoolImp::
getInstance(MINT32 i4SensorDevId)
{
    GET_PROP("vendor.debug.resultpool.log", 0, m_u4LogEn);
    switch (i4SensorDevId)
    {
        case ESensorDev_Main:
        {
            static ResultPoolImp _singleton(ESensorDev_Main);
            return &_singleton;
        }
        break;
        case ESensorDev_Sub:
        {
            static ResultPoolImp _singleton(ESensorDev_Sub);
            return &_singleton;
        }
        break;
        case ESensorDev_MainSecond:
        {
            static ResultPoolImp _singleton(ESensorDev_MainSecond);
            return &_singleton;
        }
        break;
        case ESensorDev_SubSecond:
        {
            static ResultPoolImp _singleton(ESensorDev_SubSecond);
            return &_singleton;
        }
        break;
        case ESensorDev_MainThird:
        {
            static ResultPoolImp _singleton(ESensorDev_MainThird);
            return &_singleton;
        }
        break;
        default:
            CAM_LOGE("Unsupport sensor device ID: %d\n", i4SensorDevId);
            AEE_ASSERT_3A_RESULTPOOL("Unsupport sensor device.");
            return MNULL;
    }
}

ResultPoolImp::
ResultPoolImp(MINT32 i4SensorDevId)
    : m_rResultPool(NULL)
    , m_rMetaResultPool(NULL)
    , m_u4Capacity(Capacity)
    , m_r3AImportantInfo()
    , m_i4SubsampleCount(SubsampleCount)
    , m_LockHistory()
    , m_i4CurIndex(0)
    , m_LockCurIndex()
    , m_rP2Info()
    , m_i4PreReqMagic(InvalidValue)
{
    CAM_LOGD("[%s] Constructor initial, SensorDev:%d Capacity:%d", __FUNCTION__, i4SensorDevId, m_u4Capacity);
    m_rBasicCfg.u4SensorDev = i4SensorDevId;
}

ResultPoolImp::~ResultPoolImp()
{
    CAM_LOGD("[%s] Destructor", __FUNCTION__);
}

MVOID
ResultPoolImp::
init()
{
    CAM_LOGD("[%s]+ SensorDev:%d", __FUNCTION__, m_rBasicCfg.u4SensorDev);

    //===== Clear request magic number vector =====
    {
        std::lock_guard<std::mutex> lock(m_LockHistory);
        m_vecHistory.clear();
    }

    //===== allocate metadata & result pool =====
    allocateResultPool();
    CAM_LOGD("[%s]- Create metadata and result pool done", __FUNCTION__);
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
    CAM_LOGD("[%s]+ SensorDev:%d", __FUNCTION__, m_rBasicCfg.u4SensorDev);

    //===== Clear request magic number vector =====
    {
        std::lock_guard<std::mutex> lock(m_LockHistory);
        m_vecHistory.clear();
    }

    //===== free result pool =====
    freeResultPool();

    CAM_LOGD("[%s]- free metadata and result pool done", __FUNCTION__);
}

MVOID
ResultPoolImp::
allocateResultPool()
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_OTHER) ? MTRUE: MFALSE;

    CAM_LOGD("[%s]+ SensorDev:%d Capacity:%d", __FUNCTION__, m_rBasicCfg.u4SensorDev, m_u4Capacity);

    //===== Allocate Metadata ResulPool buffer =====
    if(m_rMetaResultPool == NULL)
        m_rMetaResultPool = new AllMetaResult_T[m_u4Capacity];
    else
        CAM_LOGD("[%s] Metadata ResultPool exist", __FUNCTION__);

    //===== Debug for Metadata ResulPool buffer =====
    if(m_rMetaResultPool)
    {
        CAM_LOGD_IF(fgLog, "[%s] Create Metadata ResultPool Success", __FUNCTION__);
        for(MINT32 i4Idx = 0; i4Idx < m_u4Capacity; i4Idx++)
            CAM_LOGD_IF(fgLog, "[%s] MetaResultPool[%d]:%p", __FUNCTION__, i4Idx, &m_rMetaResultPool[i4Idx]);
    }
    else
        CAM_LOGE("[%s] Create Metadata ResultPool fail", __FUNCTION__);

    //===== Allocate ResultPool buffer =====
    if(m_rResultPool == NULL)
        m_rResultPool = new AllResult_T[m_u4Capacity];
    else
        CAM_LOGD("[%s] ResultPool exist", __FUNCTION__);

    if(m_rResultPool)
    {
        CAM_LOGD_IF(fgLog, "[%s] Create ResultPool Success", __FUNCTION__);
        for(MINT32 i4Idx = 0; i4Idx < m_u4Capacity; i4Idx++)
        {
            for(MINT32 i4EnumIdx = 0; i4EnumIdx < E_NUM_RESULT_END; i4EnumIdx++)
            {
                CAM_LOGD_IF(fgLog, "[%s] (ResultPool[%d], ModuleResult[%d]) = (%p, %p)", __FUNCTION__, i4Idx, i4EnumIdx, &m_rResultPool[i4Idx], m_rResultPool[i4Idx].ModuleResult[i4EnumIdx]);
                if(m_rResultPool[i4Idx].ModuleResult[i4EnumIdx] == NULL)
                {
                    CAM_LOGD_IF(fgLog, "[%s] Module:%d", __FUNCTION__, i4EnumIdx);
                    if(i4EnumIdx != E_NUM_RESULTTOMETA && i4EnumIdx != E_NUM_RESULT_END)
                        m_rResultPool[i4Idx].ModuleResult[i4EnumIdx] = new Entry(LOG_TAG, (E_PARTIAL_RESULT_OF_MODULE_T)i4EnumIdx);

                    if(m_rResultPool[i4Idx].ModuleResult[i4EnumIdx] != NULL)
                        CAM_LOGD_IF(fgLog, "[%s] Create Modules Result Buffer Addr Success:%p", __FUNCTION__, m_rResultPool[i4Idx].ModuleResult[i4EnumIdx]);
                    else
                        CAM_LOGD_IF(fgLog, "[%s] Create Modules Result Buffer Addr Fail - EnumIdx(%d)", __FUNCTION__, i4EnumIdx);
                }
            }
            //===== Resize vector size
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
    else
        CAM_LOGE("[%s] Ceate ResultPool Fail", __FUNCTION__);

    CAM_LOGD("[%s]-", __FUNCTION__);
}

MVOID
ResultPoolImp::
freeResultPool()
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_OTHER) ? MTRUE: MFALSE;

    CAM_LOGD("[%s]+ SensorDev:%d Capacity:%d", __FUNCTION__, m_rBasicCfg.u4SensorDev, m_u4Capacity);

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
        for(MINT32 i4Idx = 0; i4Idx < m_u4Capacity; i4Idx++)
        {
            for(MINT32 i4EnumIdx = 0; i4EnumIdx < E_NUM_RESULT_END; i4EnumIdx++)
            {
                if(m_rResultPool[i4Idx].ModuleResult[i4EnumIdx])
                {
                    CAM_LOGD_IF(fgLog, "[%s] (ResultPool[%d], ModuleResult[%d]) = (%p, %p)", __FUNCTION__, i4Idx, i4EnumIdx, &m_rResultPool[i4Idx], m_rResultPool[i4Idx].ModuleResult[i4EnumIdx]);
                    m_rResultPool[i4Idx].ModuleResult[i4EnumIdx]->free((E_PARTIAL_RESULT_OF_MODULE_T)i4EnumIdx);
                    delete m_rResultPool[i4Idx].ModuleResult[i4EnumIdx];
                    m_rResultPool[i4Idx].ModuleResult[i4EnumIdx] = NULL;
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
    {
        std::lock_guard<std::mutex> lock(m_LockCurIndex);
        m_i4CurIndex = 0;
    }

    //===== Clear request magic number vector =====
    {
        std::lock_guard<std::mutex> lock(m_LockHistory);
        m_vecHistory.clear();
    }

    //===== Clear Metadata ResulPool& ResulPool buffer =====
    if(m_rMetaResultPool && m_rResultPool)
    {
        for(MINT32 i4Idx = 0; i4Idx < m_u4Capacity; i4Idx++)
        {
            resetResultPool(i4Idx);
        }
    }

    CAM_LOGD_IF(fgLog, "[%s]-", __FUNCTION__);
}

MVOID
ResultPoolImp::
resetResultPool(const MINT32& i4Indx)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_OTHER) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s]+ Indx:%d", __FUNCTION__, i4Indx);

    //===== Clear Metadata ResulPoolmember =====
    m_rMetaResultPool[i4Indx].rMetaResult.Dummy = 0;
    m_rMetaResultPool[i4Indx].rMetaResult.MagicNum = InvalidValue;
    m_rMetaResultPool[i4Indx].i4IsConverted = E_NON_CONVERT;
    m_rResultPool[i4Indx].rRequestInfo.i4StatisticMagic = InvalidValue;

    //===== Clear ResulPool member =====
    //m_rResultPool[i4Indx].rRequestInfo.strUserName.clear();
    for(MINT32 i4EnumIdx = 0; i4EnumIdx < E_NUM_RESULT_END; i4EnumIdx++)
    {
        if(m_rResultPool[i4Indx].ModuleResult[i4EnumIdx] != NULL && i4EnumIdx != E_NUM_RESULTTOMETA && i4EnumIdx != E_NUM_RESULT_END)
        {
            m_rResultPool[i4Indx].ModuleResult[i4EnumIdx]->reset();
        }
    }

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

    CAM_LOGD_IF(fgLog, "[%s]-", __FUNCTION__);
}

MVOID
ResultPoolImp::
clearOldestResultPool(const MINT32& i4ReqMagic, const MINT32& i4Index)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_OTHER) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s]+ (ReqMagic, SubsampleCount) = (%d, %d)", __FUNCTION__, i4ReqMagic, m_i4SubsampleCount);

    if(m_i4SubsampleCount > 1) // For slow motion
    {
        // Only 32 of buffer to use, because capacity has garbage buffer.
        resetResultPool( i4Index );
    }
    else // For normal
    {
        if(i4ReqMagic == 0 && i4Index == ResultDummyIndex) // clear garbage buffer.
        {
            CAM_LOGW("[%s] garbage buffer (ReqMagic, Index) = (%d ,%d)", __FUNCTION__, i4ReqMagic, i4Index);
            resetResultPool( ResultDummyIndex );
        }
        else // Only 8 of buffer to use, because capacity has garbage buffer.
            resetResultPool( i4Index );
    }
    CAM_LOGD_IF(fgLog, "[%s]-", __FUNCTION__);
}

const MVOID*
ResultPoolImp::
getResult(const MINT32& i4ReqMagic, const E_PARTIAL_RESULT_OF_MODULE_T& eModule, const char* pFunctionName)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_GET_1) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s]+ (Function, ReqMagic, eModule) = (%s, %d, %d)", __FUNCTION__, pFunctionName, i4ReqMagic, eModule);

    //===== ReqMagic to Index =====
    MINT32 i4Index = magic2IndexRingBuffer(i4ReqMagic);
    MBOOL bIsValidate = MFALSE;

    //===== Return validate module Addr, if doesn't validate return NULL =====
    if(i4Index != -1 && m_rResultPool[i4Index].ModuleResult[eModule])
    {
        bIsValidate = m_rResultPool[i4Index].ModuleResult[eModule]->isValidate();

        CAM_LOGD_IF(fgLog, "[%s] IsValidate:%d", __FUNCTION__, bIsValidate);

        if(bIsValidate)
            return m_rResultPool[i4Index].ModuleResult[eModule]->read();
    }

    CAM_LOGD_IF(fgLog, "[%s]- Index of ReqMagic no match in ResultPool or Module doesn't validate:(%d, %d)", __FUNCTION__, i4Index, bIsValidate);
    return NULL;
}

MINT32
ResultPoolImp::
updateResult(std::string strUserName, const MINT32& i4ReqMagic, const E_PARTIAL_RESULT_OF_MODULE_T& eModule, const MVOID* pData) // pData = xxxResult_T
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_SET_1) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s]+ (ReqMagic, eModule, UserName) = (%d, %d, %s)", __FUNCTION__, i4ReqMagic, eModule, strUserName.c_str());

    //===== ReqMagic to Index =====
    MINT32 i4Index = magic2IndexRingBuffer(i4ReqMagic);
    //char dbgInfo[256];

    //===== Update module =====
    if(i4Index != -1 && m_rResultPool[i4Index].ModuleResult[eModule])
    {
        // Stash debug info //racing condition
        //sprintf(dbgInfo, "%s_%d ", strUserName.c_str(), eModule);
        //m_rResultPool[i4Index].rRequestInfo.strUserName.append(dbgInfo);

        // write data
        m_rResultPool[i4Index].ModuleResult[eModule]->write(pData);
        CAM_LOGD_IF(fgLog, "[%s]- Module:%d", __FUNCTION__,eModule);
        return MTRUE;
    }

    CAM_LOGD_IF(fgLog, "[%s]- Index of ReqMagic no match in ResultPool:%d", __FUNCTION__, i4Index);
    return MFALSE;
}

AllResult_T*
ResultPoolImp::
getResultByReqNum(const MINT32& i4ReqMagic, const char* pFunctionName)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_LOCK) ? MTRUE: MFALSE;

    //===== ReqMagic to Index =====
    MINT32 i4Index = magic2IndexRingBuffer(i4ReqMagic);

    if(i4Index == -1)
        return NULL;

    CAM_LOGD_IF(fgLog, "[%s] Lock + : (1) index(%d)/address(%p)/Req(%d)/func(%s)", __FUNCTION__, i4Index, &m_rResultPool[i4Index], i4ReqMagic, pFunctionName);
    m_rResultPool[i4Index].LockVecResult.lock();
    CAM_LOGD_IF(fgLog, "[%s] Lock   : (2)", __FUNCTION__);
    return &m_rResultPool[i4Index];
}

VOID
ResultPoolImp::
returnResult(AllResult_T* pAllResult, const char* pFunctionName)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_LOCK) ? MTRUE: MFALSE;
    if(pAllResult != NULL){
        CAM_LOGD_IF(fgLog, "[%s] Lock - : (3) address(%p)/Req(%d)/Stt(%d)/Preset(%d)/func(%s)", __FUNCTION__,
            pAllResult,
            pAllResult->rRequestInfo.i4ReqMagic,
            pAllResult->rRequestInfo.i4StatisticMagic,
            pAllResult->rRequestInfo.i4PreSetKey,
            pFunctionName);

        pAllResult->LockVecResult.unlock();
        pAllResult = NULL;
    }
    return;
}

MINT32
ResultPoolImp::
setCapacity(const MUINT32& u4Capacity, const MINT32& i4SubsampleCount)
{
    CAM_LOGD("[%s]+ (Src-Capacity, Dst-Capacity, SubsampleCount) = (%d, %d, %d)", __FUNCTION__, u4Capacity, m_u4Capacity, i4SubsampleCount);

    //===== normal change to slow motio, re-allocate resultPool buffer =====
    if(u4Capacity != m_u4Capacity)
    {
        //Need to release old buffer with SMVR
        freeResultPool();

        m_u4Capacity = u4Capacity;
        m_i4SubsampleCount = i4SubsampleCount;
        allocateResultPool();
        CAM_LOGD("[%s]- ", __FUNCTION__);
        return MTRUE;
    }
    CAM_LOGD("[%s]- ", __FUNCTION__);
    return MFALSE;
}

MINT32
ResultPoolImp::
isValidateByReqNum(const MINT32& i4ReqMagic, const E_PARTIAL_RESULT_OF_MODULE_T& eModule)
{
    CAM_LOGD_IF(m_u4LogEn, "[%s]+ (ReqMagic, SensorDev) = (%d, %d)", __FUNCTION__, i4ReqMagic, m_rBasicCfg.u4SensorDev);

    //===== ReqMagic to Index =====
    MINT32 i4Index = magic2IndexRingBuffer(i4ReqMagic);
    MINT32 i4Validate = MFALSE;

    if(i4Index != -1)
    {
        i4Validate = m_rResultPool[i4Index].ModuleResult[eModule]->isValidate();
    }
    CAM_LOGD_IF( (i4Validate == MFALSE || i4Index == -1), "[%s]- (ReqMagic, SensorDev) = (%d, %d), Index of ReqMagic no match in ResultPool or Validate:(%d, %d)", __FUNCTION__, i4ReqMagic, m_rBasicCfg.u4SensorDev, i4Index, i4Validate);
    return i4Validate;
}

MINT32
ResultPoolImp::
isValidateBySttNum(const MINT32& i4SttMagic, const E_PARTIAL_RESULT_OF_MODULE_T& eModule)
{
    CAM_LOGD("[%s]+ (SttMagic, SensorDev) = (%d, %d)", __FUNCTION__, i4SttMagic, m_rBasicCfg.u4SensorDev);

    MINT32 i4Validate = MFALSE;
    MINT32 i4Index = -1;

    //===== Find SttMagic index =====
    for(MINT32 i = 0; i < m_u4Capacity; i++)
    {
        if(m_rResultPool[i].rRequestInfo.i4StatisticMagic == i4SttMagic)
        {
            i4Index = i;
            CAM_LOGD("[%s] (ResultSttMagic, Index , SttMagic) = (%d, %d, %d)", __FUNCTION__, m_rResultPool[i].rRequestInfo.i4StatisticMagic, i4Index, i4SttMagic);
        }
    }

    //===== Check basic module enum validate or not =====
    if(i4Index != -1)
    {
        i4Validate = m_rResultPool[i4Index].ModuleResult[eModule]->isValidate();
    }

    CAM_LOGD("[%s]- Index of SttMagic no match in ResultPool or Validate:(%d, %d)", __FUNCTION__, i4Index, i4Validate);
    return i4Validate;
}

MBOOL
ResultPoolImp::
findValidateBuffer(MINT32 i4ReqMagic)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_SET_0) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s]+ ReqMagic:%d", __FUNCTION__, i4ReqMagic);

    //===== ReqMagic to Index =====
    MINT32 i4Index = getValidateIndex(i4ReqMagic);

    if(i4Index != -1)
    {
        clearOldestResultPool(i4ReqMagic, i4Index);

        //===== update Requst magicNum to ResultPool =====
        m_rResultPool[i4Index].rRequestInfo.i4ReqMagic = i4ReqMagic;
        m_rMetaResultPool[i4Index].rMetaResult.MagicNum = i4ReqMagic;

        CAM_LOGD_IF(fgLog, "[%s]- Index:%d", __FUNCTION__, i4Index);
        return MTRUE;
    } else
        return MFALSE;
}

AllMetaResult_T*
ResultPoolImp::
getMetadata(const MINT32& i4ReqMagic, const char* pFunctionName)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_LOCK) ? MTRUE: MFALSE;

    //===== ReqMagic to Index =====
    MINT32 i4Index = magic2IndexRingBuffer(i4ReqMagic);

    if(i4Index == -1)
        return NULL;

    CAM_LOGD_IF(fgLog, "[%s] Lock + : (1) index(%d)/address(%p)/Req(%d)/func(%s)", __FUNCTION__, i4Index, &m_rMetaResultPool[i4Index], i4ReqMagic, pFunctionName);
    m_rMetaResultPool[i4Index].LockMetaResult.lock();
    CAM_LOGD_IF(fgLog, "[%s] Lock   : (2)", __FUNCTION__);

    return &(m_rMetaResultPool[i4Index]);
}

MVOID
ResultPoolImp::
returnMetadata(AllMetaResult_T* pMetaResult, const char* pFunctionName)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_LOCK) ? MTRUE: MFALSE;
    if(pMetaResult != NULL){
        CAM_LOGD_IF(fgLog, "[%s] Lock - : (3) address(%p)/Req(%d)/func(%s)", __FUNCTION__,
            pMetaResult,
            pMetaResult->rMetaResult.MagicNum,
            pFunctionName);

        pMetaResult->LockMetaResult.unlock();
        pMetaResult = NULL;
    }
    return;
}

MVOID
ResultPoolImp::
setHistory(const MINT32& i4ReqMagic, const MINT32& Size)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_SET_0) ? MTRUE: MFALSE;

    std::lock_guard<std::mutex> lock(m_LockHistory);

    m_vecHistory.push_back(i4ReqMagic);

    if(m_vecHistory.size() > Size)
        m_vecHistory.erase(m_vecHistory.begin());

    //===== For Debug =====
    for(MINT32 i = 0; i < m_vecHistory.size(); i++)
        CAM_LOGD_IF(fgLog,"[%s]History = (%d, %d, %d, %d)", __FUNCTION__, m_vecHistory.size(), m_vecHistory[0], m_vecHistory[1], m_vecHistory[2]);
}

MINT32
ResultPoolImp::
getHistory(MINT32* pHistory)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_GET_0) ? MTRUE: MFALSE;

    std::lock_guard<std::mutex> lock(m_LockHistory);

    CAM_LOGD_IF(fgLog,"[%s]m_vecHistory.empty() %d", __FUNCTION__, m_vecHistory.empty());

    if(m_vecHistory.size() == 0)
    {
        CAM_LOGD("[%s]m_vecHistory.size() %d", __FUNCTION__, m_vecHistory.size());
        return MFALSE;
    }

    //===== For Debug =====
    if(!m_vecHistory.empty())
    {
        for(MINT32 i = 0; i < m_vecHistory.size(); i++)
            CAM_LOGD_IF(fgLog,"[%s]History = (%d, %d, %d, %d)", __FUNCTION__, m_vecHistory.size(), m_vecHistory[0], m_vecHistory[1], m_vecHistory[2]);
    }

    //===== Assigne to out buffer =====
    for(MINT32 i = 0; i < m_vecHistory.size(); i++)
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
getValidateIndex(const MINT32& i4ReqMagic)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_PF) ? MTRUE: MFALSE;

    std::lock_guard<std::mutex> lock(m_LockCurIndex);
    CAM_LOGD_IF(fgLog, "[%s]+ ReqMagic:%d, PreReqMagic:%d, SubsampleCount:%d, m_i4CurIndex:%d", __FUNCTION__, i4ReqMagic, m_i4PreReqMagic, m_i4SubsampleCount, m_i4CurIndex);
    MINT32 i4Index = 0;

    //===== ReqMagic to Index =====
    if(m_i4SubsampleCount > 1) // For slow motion
    {
        if(m_i4PreReqMagic != i4ReqMagic)
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

    m_i4PreReqMagic = i4ReqMagic;
    CAM_LOGD_IF(fgLog, "[%s] - Final Index(%d)", __FUNCTION__, i4Index);

    return i4Index;
}

MINT32
ResultPoolImp::
magic2IndexRingBuffer(const MINT32& i4ReqMagic)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_PF) ? MTRUE: MFALSE;

    std::lock_guard<std::mutex> lock(m_LockCurIndex);
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
            CAM_LOGD_IF(fgLog, "[%s] (i4Index, Local-i4CurIndex, Golbal-i4CurIndex) = (%d, %d, %d) MagicNum(%d) ", __FUNCTION__, i4Index, i4CurIndex, m_i4CurIndex, m_rResultPool[i4Index].rRequestInfo.i4ReqMagic);
#if 0 // Because config stage doesn't have metadata, not use magicNum of matadata to judge
            if(m_rMetaResultPool[i4Index].rMetaResult.MagicNum == i4ReqMagic && m_rResultPool[i4Index].rRequestInfo.i4ReqMagic == i4ReqMagic)
#else
            if(m_rResultPool[i4Index].rRequestInfo.i4ReqMagic == i4ReqMagic)
#endif
                break;
            i4Index = (i4Index - 1 + i4BufferSize)% i4BufferSize;
        }
#if 0 // Because config stage doesn't have metadata, not use magicNum of matadata to judge
        if(i4Index == i4CurIndex && m_rMetaResultPool[i4Index].rMetaResult.MagicNum != i4ReqMagic)
#else
        if(i4Index == i4CurIndex && m_rResultPool[i4Index].rRequestInfo.i4ReqMagic != i4ReqMagic)
#endif
            i4Index = -1;
    }
    else // For normal
    {
        i4Index = (i4CurIndex - 1 + ResultNum)% ResultNum;
        while(i4Index != i4CurIndex)
        {
            CAM_LOGD_IF(fgLog, "[%s] (i4Index, Local-i4CurIndex, Golbal-i4CurIndex) = (%d, %d, %d) MagicNum(%d) ", __FUNCTION__, i4Index, i4CurIndex, m_i4CurIndex, m_rResultPool[i4Index].rRequestInfo.i4ReqMagic);
#if 0 // Because config stage doesn't have metadata, not use magicNum of matadata to judge
            if(m_rMetaResultPool[i4Index].rMetaResult.MagicNum == i4ReqMagic && m_rResultPool[i4Index].rRequestInfo.i4ReqMagic == i4ReqMagic)
#else
            if(m_rResultPool[i4Index].rRequestInfo.i4ReqMagic == i4ReqMagic)
#endif
                break;
            i4Index = (i4Index - 1 + ResultNum)% ResultNum;
        }
#if 0 // Because config stage doesn't have metadata, not use magicNum of matadata to judge
        if(i4Index == i4CurIndex && m_rMetaResultPool[i4Index].rMetaResult.MagicNum != i4ReqMagic)
#else
        if(i4Index == i4CurIndex && m_rResultPool[i4Index].rRequestInfo.i4ReqMagic != i4ReqMagic)
#endif
            i4Index = -1;
    }

    CAM_LOGD_IF(fgLog, "[%s] - Final Index(%d)", __FUNCTION__, i4Index);

    return i4Index;
}


REQUEST_INFO_T
ResultPoolImp::
getRequestInfo(const MINT32& i4ReqMagic)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_GET_0) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s]+ ReqMagic:%d", __FUNCTION__, i4ReqMagic);

    //===== ReqMagic to Index =====
    MINT32 i4Index = magic2IndexRingBuffer(i4ReqMagic);

    if(i4Index == -1)
    {
        CAM_LOGW("[%s]- Index is illegal value(%d)", __FUNCTION__, i4Index);
        return REQUEST_INFO_T();
    }

    CAM_LOGD_IF(fgLog, "[%s]- i4Index:%d", __FUNCTION__, i4Index);

    return m_rResultPool[i4Index].rRequestInfo;
}
MBOOL
ResultPoolImp::
setRequestInfo(const REQUEST_INFO_T& rRequestInfo)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_GET_0) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s]+ (ReqMagic, SttMagic) = (%d, %d)", __FUNCTION__, rRequestInfo.i4ReqMagic, rRequestInfo.i4StatisticMagic);

    //===== ReqMagic to Index =====
    MINT32 i4Index = magic2IndexRingBuffer(rRequestInfo.i4ReqMagic);

    if(i4Index != -1)
    {
        ::memcpy(&(m_rResultPool[i4Index].rRequestInfo), &rRequestInfo, sizeof(REQUEST_INFO_T));
        return MTRUE;
    }

    CAM_LOGD("[%s]- (ReqMagic, SttMagic) = (%d, %d), Index of ReqMagic no match in ResultPool:%d", __FUNCTION__, rRequestInfo.i4ReqMagic, rRequestInfo.i4StatisticMagic, i4Index);
    return MFALSE;
}

AllResult_T*
ResultPoolImp::
getResultBySttNum(const MINT32& i4SttMagic, const char* pFunctionName)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_LOCK) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s]+ SttMagic:%d", __FUNCTION__, i4SttMagic);

    //===== Find capture index of stt megic =====
    for(MINT32 i4Index = 0; i4Index < m_u4Capacity; i4Index++)
    {
        if(m_rResultPool[i4Index].rRequestInfo.i4StatisticMagic == i4SttMagic)
        {
            CAM_LOGD("[%s] (Index, ResultStt) = (%d, %d)", __FUNCTION__, i4Index, m_rResultPool[i4Index].rRequestInfo.i4StatisticMagic);
            return getResultByReqNum(m_rResultPool[i4Index].rRequestInfo.i4ReqMagic, pFunctionName);
        }
        else
            CAM_LOGD_IF(fgLog, "[%s] Fail (Src-ResultStt, Dst-ResultStt, Index) = (%d, %d, %d)", __FUNCTION__, i4SttMagic, m_rResultPool[i4Index].rRequestInfo.i4StatisticMagic, i4Index);
    }

    CAM_LOGD_IF(fgLog, "[%s]-", __FUNCTION__);
    return NULL;
}

AllResult_T*
ResultPoolImp::
getResultByPresetKey(MINT32 i4PreSetKey, const char* pFunctionName)
{
    MBOOL fgLog = (m_u4LogEn & HAL3RESULTPOOL_LOG_LOCK) ? MTRUE: MFALSE;

    CAM_LOGD_IF(fgLog, "[%s]+ PresetKey:%d", __FUNCTION__, i4PreSetKey);

    //===== Find capture index of stt megic =====
    for(MUINT32 i4Index = 0; i4Index < m_u4Capacity; i4Index++)
    {
        if(m_rResultPool[i4Index].rRequestInfo.i4PreSetKey == i4PreSetKey)
        {
            CAM_LOGD_IF(fgLog, "[%s] Lock + : (1) index(%d)/address(%p)/Req(%d)/Preset(%d)/func(%s)", __FUNCTION__,
                i4Index, &m_rResultPool[i4Index],
                m_rResultPool[i4Index].rRequestInfo.i4ReqMagic,
                m_rResultPool[i4Index].rRequestInfo.i4PreSetKey,
                pFunctionName);
            m_rResultPool[i4Index].LockVecResult.lock();
            CAM_LOGD_IF(fgLog, "[%s] Lock   : (2)", __FUNCTION__);
            return &m_rResultPool[i4Index];
        }
        else
            CAM_LOGD_IF(fgLog, "[%s] Fail (In-PreSetKey, Local-PreSetKey, Index) = (%d, %d, %d)", __FUNCTION__, i4PreSetKey, m_rResultPool[i4Index].rRequestInfo.i4PreSetKey, i4Index);
    }

    CAM_LOGD_IF(fgLog, "[%s]-", __FUNCTION__);
    return NULL;
}

MVOID
ResultPoolImp::
updateP2Info(P2Info_T& rP2Info)
{
    m_rP2Info.rSclCropRectl.p.x = rP2Info.rSclCropRectl.p.x;
    m_rP2Info.rSclCropRectl.p.y = rP2Info.rSclCropRectl.p.y;
    m_rP2Info.rSclCropRectl.s.w = rP2Info.rSclCropRectl.s.w;
    m_rP2Info.rSclCropRectl.s.h = rP2Info.rSclCropRectl.s.h;

    m_rP2Info.AF_GMV_X      = rP2Info.AF_GMV_X;
    m_rP2Info.AF_GMV_Y      = rP2Info.AF_GMV_Y;
    m_rP2Info.AF_GMV_Conf_X = rP2Info.AF_GMV_Conf_X;
    m_rP2Info.AF_GMV_Conf_Y = rP2Info.AF_GMV_Conf_Y;
    m_rP2Info.AF_GMV_Max    = rP2Info.AF_GMV_Max;
}
MVOID
ResultPoolImp::
getP2Info(P2Info_T& rP2Info)
{
    rP2Info.rSclCropRectl.p.x = m_rP2Info.rSclCropRectl.p.x;
    rP2Info.rSclCropRectl.p.y = m_rP2Info.rSclCropRectl.p.y;
    rP2Info.rSclCropRectl.s.w = m_rP2Info.rSclCropRectl.s.w;
    rP2Info.rSclCropRectl.s.h = m_rP2Info.rSclCropRectl.s.h;

    rP2Info.AF_GMV_X      = m_rP2Info.AF_GMV_X;
    rP2Info.AF_GMV_Y      = m_rP2Info.AF_GMV_Y;
    rP2Info.AF_GMV_Conf_X = m_rP2Info.AF_GMV_Conf_X;
    rP2Info.AF_GMV_Conf_Y = m_rP2Info.AF_GMV_Conf_Y;
    rP2Info.AF_GMV_Max    = m_rP2Info.AF_GMV_Max;
}

