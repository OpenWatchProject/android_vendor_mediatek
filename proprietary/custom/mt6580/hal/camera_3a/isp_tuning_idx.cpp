/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#define LOG_TAG "isp_tuning_idx"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <string.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include <aaa_log.h>
#include <aaa_types.h>
#include "camera_custom_nvram.h"
#include <isp_tuning.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_idx.h>
#include <cfg_isp_tuning_idx_macro.h>
#include "cfg_isp_tuning_idx.h"


using namespace NSIspTuning;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
namespace
{
    enum EIndexNum
    {
        NUM_OBC         =   NVRAM_OBC_TBL_NUM,
        NUM_BPC         =   NVRAM_BPC_TBL_NUM,
        NUM_NR1         =   NVRAM_NR1_TBL_NUM,
        NUM_CFA         =   NVRAM_CFA_TBL_NUM,
        NUM_GGM         =   NVRAM_GGM_TBL_NUM,
        NUM_ANR         =   NVRAM_ANR_TBL_NUM,
        NUM_CCR         =   NVRAM_CCR_TBL_NUM,
        NUM_EE          =   NVRAM_EE_TBL_NUM,
    };

    template <EIndexNum Num>
    inline MBOOL setIdx(UINT16 &rIdxTgt, UINT16 const IdxSrc)
    {
        if  (IdxSrc < Num)
        {
            rIdxTgt = IdxSrc;
            return  MTRUE;
        }
        return  MFALSE;
    }

};  //  namespace


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IndexMgr::setIdx_OBC(UINT16 const idx)   { return setIdx<NUM_OBC>(OBC, idx); }
MBOOL IndexMgr::setIdx_BPC(UINT16 const idx)   { return setIdx<NUM_BPC>(BPC, idx); }
MBOOL IndexMgr::setIdx_NR1(UINT16 const idx)   { return setIdx<NUM_NR1>(NR1, idx); }
MBOOL IndexMgr::setIdx_CFA(UINT16 const idx)   { return setIdx<NUM_CFA>(CFA, idx); }
MBOOL IndexMgr::setIdx_GGM(UINT16 const idx)   { return setIdx<NUM_GGM>(GGM, idx); }
MBOOL IndexMgr::setIdx_ANR(UINT16 const idx)   { return setIdx<NUM_ANR>(ANR, idx); }
MBOOL IndexMgr::setIdx_CCR(UINT16 const idx)   { return setIdx<NUM_CCR>(CCR, idx); }
MBOOL IndexMgr::setIdx_EE(UINT16 const idx)    { return setIdx<NUM_EE>(EE, idx);   }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
IndexMgr::
dump() const
{
    MY_LOG(
        "[IndexMgr][dump]"
        " OBC:%d, BPC:%d, NR1:%d, CFA:%d, GGM:%d, ANR:%d, CCR:%d, EE:%d"
        , OBC, BPC, NR1, CFA, GGM, ANR, CCR, EE
    );
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IdxSetMgrBase&
IdxSetMgrBase::
getInstance()
{
    static IdxSetMgr singleton;

    static struct link
    {
        link(IdxSetMgr& r)
        {
            r.init();
            r.linkIndexSet_preview();
            r.linkIndexSet_capture();
            r.linkIndexSet_video();
        }
    } link_singleton(singleton);

    return  singleton;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IdxSetMgr::
init()
{
    ::memset(m_pNormalPreview,  0, sizeof(m_pNormalPreview));
    ::memset(m_pZsdPreview_CC,  0, sizeof(m_pZsdPreview_CC));
    ::memset(m_pZsdPreview_NCC,  0, sizeof(m_pZsdPreview_NCC));
    ::memset(m_pNormalCapture,  0, sizeof(m_pNormalCapture));
    ::memset(m_pVideoPreview,   0, sizeof(m_pVideoPreview));
    ::memset(m_pVideoCapture,   0, sizeof(m_pVideoCapture));
    ::memset(m_pMFCapturePass1, 0, sizeof(m_pMFCapturePass1));
    ::memset(m_pMFCapturePass2, 0, sizeof(m_pMFCapturePass2));

    //======================================================================
    //LINK_SCENEs_ISOs(LINK_NORMAL_PREVIEW);
    //LINK_SCENEs_ISOs(LINK_ZSD_PREVIEW_CC);
    //LINK_SCENEs_ISOs(LINK_ZSD_PREVIEW_NCC);
    //LINK_SCENEs_ISOs(LINK_NORMAL_CAPTURE);
    //======================================================================

    //======================================================================
    //LINK_ONE_SCENE_ISOs(LINK_VIDEO_PREVIEW, -1);
    //LINK_ONE_SCENE_ISOs(LINK_VIDEO_CAPTURE, -1);
    //LINK_ONE_SCENE_ISOs(LINK_MF_CAPTURE_PASS1, -1);
    //LINK_ONE_SCENE_ISOs(LINK_MF_CAPTURE_PASS2, -1);
    //======================================================================
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
INDEX_T const*
IdxSetMgr::
get(MUINT32 ispProfile, MUINT32 sensor/*=-1*/, MUINT32 const scene/*=-1*/, MUINT32 const iso/*=-1*/) const
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.enable.isp.profile", value, "0");
    MBOOL bEnableIspProfileSetting = atoi(value);

    // for AE1 video mode tuning purpose
    if (bEnableIspProfileSetting) {
        property_get("vendor.set.isp.profile", value, "1");
        MINT32 i4NewIspProfile = atoi(value);
        ispProfile = static_cast<MUINT32>(i4NewIspProfile);
        MY_LOG("i4NewIspProfile = %d", i4NewIspProfile);
    }

    // Add for new sensor mode after MP
#if 0
    if (sensor > ESensorMode_SlimVideo2) { // sensor mode re-mapping for ISP tuning paramter sharing
        sensor = static_cast<MUINT32>(ESensorMode_SlimVideo2);
    }
#endif

    switch  (ispProfile)
    {
/*
NOTE:
1. Based on R-like tuning,
   the following profiles all map to the same tuning idx set (capture)
   Capture, ZSD, VSS, Pure RAW Capture
2. Based on R-like tuning,
   sensor mode has been taken into consideration
   -> EIspProfile_VideoPreview and EIspProfile_VideoCapture map to the same tuning idx set (video)
 */
    //  Normal
    case EIspProfile_NormalPreview:
        return  get_NormalPreview(sensor, scene, iso);
    case EIspProfile_ZsdPreview_CC:
        return  get_NormalCapture(sensor, scene, iso);//ISP_TEMP_MARK_OUT get_ZsdPreview_CC(sensor, scene, iso);
    case EIspProfile_ZsdPreview_NCC:
        return  get_NormalCapture(sensor, scene, iso);//ISP_TEMP_MARK_OUT get_ZsdPreview_NCC(sensor, scene, iso);
    case EIspProfile_NormalCapture:
    case EIspProfile_NormalCapture_CC:
    case EIspProfile_NormalCapture_16M:
        return  get_NormalCapture(sensor, scene, iso);//ISP_TEMP_MARK_OUT get_NormalCapture(sensor, scene, iso);
    case EIspProfile_VideoPreview:
        return  get_VideoPreview(sensor, scene, iso);//ISP_TEMP_MARK_OUT get_VideoPreview(sensor, scene, iso);
    case EIspProfile_VideoCapture:
        return  get_VideoPreview(sensor, scene, iso);//ISP_TEMP_MARK_OUT get_VideoCapture(sensor, scene, iso);
    case EIspProfile_MFCapPass1:
        return  get_NormalCapture(sensor, scene, iso);//ISP_TEMP_MARK_OUT get_MFCapturePass1(sensor, scene, iso);
    case EIspProfile_MFCapPass2:
        return  get_NormalCapture(sensor, scene, iso);//ISP_TEMP_MARK_OUT get_MFCapturePass2(sensor, scene, iso);
    default:
        break;
    }
    return  NULL;
}


