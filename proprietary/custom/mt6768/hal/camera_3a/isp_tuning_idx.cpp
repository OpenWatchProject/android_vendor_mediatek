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

#include "cfg_isp_tuning_idx_feature.h"
#include "cfg_isp_tuning_idx_macro.h"

using namespace NSIspTuning;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
namespace
{
    enum EIndexNum
    {
        NUM_DBS         =   NVRAM_DBS_TBL_NUM,
        NUM_OBC         =   NVRAM_OBC_TBL_NUM,
        NUM_BPC         =   NVRAM_BPC_TBL_NUM,
        NUM_NR1         =   NVRAM_NR1_TBL_NUM,
        NUM_PDC         =   NVRAM_PDC_TBL_NUM,
//        NUM_RMM         =   NVRAM_RMM_TBL_NUM,
//        NUM_HLR         =   NVRAM_HLR_TBL_NUM,
        NUM_RNR         =   NVRAM_RNR_TBL_NUM,
        NUM_UDM         =   NVRAM_UDM_TBL_NUM,
        NUM_ANR         =   NVRAM_ANR_TBL_NUM,
        NUM_ANR2        =   NVRAM_ANR2_TBL_NUM,
//        NUM_ABF         =   NVRAM_ABF_TBL_NUM,
        NUM_CCR         =   NVRAM_CCR_TBL_NUM,
//        NUM_BOK         =   NVRAM_BOK_TBL_NUM,
        NUM_HFG         =   NVRAM_HFG_TBL_NUM,
        NUM_EE          =   NVRAM_EE_TBL_NUM,
        NUM_NR3D        =   NVRAM_NR3D_TBL_NUM,
        NUM_MFB         =   NVRAM_MFB_TBL_NUM,
        NUM_MIX3        =   NVRAM_MIXER3_TBL_NUM,
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
MBOOL IndexMgr::setIdx_DBS(UINT16 const idx)   { return setIdx<NUM_DBS>(DBS, idx); }
MBOOL IndexMgr::setIdx_OBC(UINT16 const idx)   { return setIdx<NUM_OBC>(OBC, idx); }
MBOOL IndexMgr::setIdx_BPC(UINT16 const idx)   { return setIdx<NUM_BPC>(BNR_BPC, idx); }
MBOOL IndexMgr::setIdx_NR1(UINT16 const idx)   { return setIdx<NUM_NR1>(BNR_NR1, idx); }
MBOOL IndexMgr::setIdx_PDC(UINT16 const idx)   { return setIdx<NUM_PDC>(BNR_PDC, idx); }
//MBOOL IndexMgr::setIdx_RMM(UINT16 const idx)   { return setIdx<NUM_RMM>(RMM, idx);   }
//MBOOL IndexMgr::setIdx_HLR(UINT16 const idx)   { return setIdx<NUM_HLR>(HLR, idx);   }
MBOOL IndexMgr::setIdx_RNR(UINT16 const idx)   { return setIdx<NUM_RNR>(RNR, idx);   }
MBOOL IndexMgr::setIdx_UDM(UINT16 const idx)   { return setIdx<NUM_UDM>(UDM, idx); }
MBOOL IndexMgr::setIdx_ANR(UINT16 const idx)   { return setIdx<NUM_ANR>(ANR, idx); }
MBOOL IndexMgr::setIdx_ANR2(UINT16 const idx)   { return setIdx<NUM_ANR2>(ANR2, idx); }
//MBOOL IndexMgr::setIdx_ABF(UINT16 const idx)   { return setIdx<NUM_ABF>(ABF, idx);   }
MBOOL IndexMgr::setIdx_CCR(UINT16 const idx)   { return setIdx<NUM_CCR>(CCR, idx); }
//MBOOL IndexMgr::setIdx_BOK(UINT16 const idx)   { return setIdx<NUM_BOK>(BOK, idx); }
MBOOL IndexMgr::setIdx_HFG(UINT16 const idx)    { return setIdx<NUM_HFG>(HFG, idx);   }
MBOOL IndexMgr::setIdx_EE(UINT16 const idx)    { return setIdx<NUM_EE>(EE, idx);   }
MBOOL IndexMgr::setIdx_NR3D(UINT16 const idx)  { return setIdx<NUM_NR3D>(NR3D, idx); }
MBOOL IndexMgr::setIdx_MFB(UINT16 const idx)   { return setIdx<NUM_MFB>(MFB, idx);   }
MBOOL IndexMgr::setIdx_MIX3(UINT16 const idx)   { return setIdx<NUM_MIX3>(MIXER3, idx);   }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
IndexMgr::
dump() const
{
    MY_LOG(
        "[IndexMgr][dump]"
        " DBS:%d, OBC:%d, BPC:%d, NR1:%d, PDC:%d,    RNR:%d, UDM:%d, ANR:%d, ANR2:%d, CCR:%d, HFG:%d, EE:%d, MFB:%d, MIX3:%d"
        , DBS,    OBC,    BNR_BPC,BNR_NR1,BNR_PDC,   RNR,    UDM,    ANR,    ANR2,    CCR,    HFG,    EE,    MFB,    MIXER3
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
            r.linkIndexSet_video();
            r.linkIndexSet_capture();

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
    ::memset(m_pPreview,                0, sizeof(m_pPreview));
    ::memset(m_pPreview_Capture_Zoom1, 0, sizeof(m_pPreview_Capture_Zoom1));
    ::memset(m_pPreview_Capture_Zoom2, 0, sizeof(m_pPreview_Capture_Zoom2));
    ::memset(m_pVideo,                  0, sizeof(m_pVideo));
    ::memset(m_pVideo_Preview_Zoom1,    0, sizeof(m_pVideo_Preview_Zoom1));
    ::memset(m_pVideo_Preview_Zoom2,    0, sizeof(m_pVideo_Preview_Zoom2));
    ::memset(m_pVideo_Video_Zoom1,      0, sizeof(m_pVideo_Video_Zoom1));
    ::memset(m_pVideo_Video_Zoom2,      0, sizeof(m_pVideo_Video_Zoom2));
    ::memset(m_pCapture,                0, sizeof(m_pCapture));
    ::memset(m_pCapture_Capture_Zoom1,  0, sizeof(m_pCapture_Capture_Zoom1));
    ::memset(m_pCapture_Capture_Zoom2,  0, sizeof(m_pCapture_Capture_Zoom2));
    ::memset(m_pHDR_Preview,            0, sizeof(m_pHDR_Preview));
    ::memset(m_pHDR_Video,              0, sizeof(m_pHDR_Video));
    ::memset(m_pHDR_Preview_VSS,        0, sizeof(m_pHDR_Preview_VSS));
    ::memset(m_pHDR_Video_VSS,          0, sizeof(m_pHDR_Video_VSS));
    ::memset(m_pHDR_Capture,            0, sizeof(m_pHDR_Capture));
    ::memset(m_pAuto_HDR_Preview,       0, sizeof(m_pAuto_HDR_Preview));
    ::memset(m_pAuto_HDR_Video,         0, sizeof(m_pAuto_HDR_Video));
    ::memset(m_pAuto_HDR_Preview_VSS,   0, sizeof(m_pAuto_HDR_Preview_VSS));
    ::memset(m_pAuto_HDR_Video_VSS,     0, sizeof(m_pAuto_HDR_Video_VSS));
    ::memset(m_pAuto_HDR_Capture,       0, sizeof(m_pAuto_HDR_Capture));
    ::memset(m_pMFNR_Before,            0, sizeof(m_pMFNR_Before));
    ::memset(m_pMFNR_Before_Zoom0,      0, sizeof(m_pMFNR_Before_Zoom0));
    ::memset(m_pMFNR_Before_Zoom1,      0, sizeof(m_pMFNR_Before_Zoom1));
    ::memset(m_pMFNR_Before_Zoom2,      0, sizeof(m_pMFNR_Before_Zoom2));
    ::memset(m_pMFNR_Single,            0, sizeof(m_pMFNR_Single));
    ::memset(m_pMFNR_Single_Zoom0,      0, sizeof(m_pMFNR_Single_Zoom0));
    ::memset(m_pMFNR_Single_Zoom1,      0, sizeof(m_pMFNR_Single_Zoom1));
    ::memset(m_pMFNR_Single_Zoom2,      0, sizeof(m_pMFNR_Single_Zoom1));
    ::memset(m_pMFNR_MFB,               0, sizeof(m_pMFNR_MFB));
    ::memset(m_pMFNR_MFB_Zoom0,         0, sizeof(m_pMFNR_MFB_Zoom0));
    ::memset(m_pMFNR_MFB_Zoom1,         0, sizeof(m_pMFNR_MFB_Zoom1));
    ::memset(m_pMFNR_MFB_Zoom2,         0, sizeof(m_pMFNR_MFB_Zoom2));
    ::memset(m_pMFNR_After,             0, sizeof(m_pMFNR_After));
    ::memset(m_pMFNR_After_Zoom0,       0, sizeof(m_pMFNR_After_Zoom0));
    ::memset(m_pMFNR_After_Zoom1,       0, sizeof(m_pMFNR_After_Zoom1));
    ::memset(m_pMFNR_After_Zoom2,       0, sizeof(m_pMFNR_After_Zoom2));
    ::memset(m_pMFNR_Before_zCap,       0, sizeof(m_pMFNR_Before_zCap));
    ::memset(m_pMFNR_Single_zCap,       0, sizeof(m_pMFNR_Single_zCap));
    ::memset(m_pMFNR_MFB_zCap,          0, sizeof(m_pMFNR_MFB_zCap));
    ::memset(m_pMFNR_After_zCap,        0, sizeof(m_pMFNR_After_zCap));
    ::memset(m_pEIS_Preview,            0, sizeof(m_pEIS_Preview));
    ::memset(m_pEIS_Video,              0, sizeof(m_pEIS_Video));
    ::memset(m_pEIS_HDR_Preview,        0, sizeof(m_pEIS_HDR_Preview));
    ::memset(m_pEIS_HDR_Video,          0, sizeof(m_pEIS_HDR_Video));
    ::memset(m_pEIS_Auto_HDR_Preview,   0, sizeof(m_pEIS_Auto_HDR_Preview));
    ::memset(m_pEIS_Auto_HDR_Video,     0, sizeof(m_pEIS_Auto_HDR_Video));
    ::memset(m_pMNR,                    0, sizeof(m_pMNR));
    ::memset(m_pYUV_Reprocess,          0, sizeof(m_pYUV_Reprocess));
    ::memset(m_pFlash_Cap,              0, sizeof(m_pFlash_Cap));
    ::memset(m_pFlash_Cap_Zoom1,        0, sizeof(m_pFlash_Cap_Zoom1));
    ::memset(m_pFlash_Cap_Zoom1,        0, sizeof(m_pFlash_Cap_Zoom2));
    ::memset(m_pN3D_Preview,            0, sizeof(m_pN3D_Preview));
    ::memset(m_pN3D_Video,              0, sizeof(m_pN3D_Video));
    ::memset(m_pN3D_Capture,            0, sizeof(m_pN3D_Capture));
    ::memset(m_pN3D_Denoise,            0, sizeof(m_pN3D_Denoise));\
    ::memset(m_pN3D_Denoise_toGGM,               0, sizeof(m_pN3D_Denoise_toGGM           ));
    ::memset(m_pN3D_Denoise_toYUV,               0, sizeof(m_pN3D_Denoise_toYUV           ));
    ::memset(m_pN3D_Denoise_toW,                 0, sizeof(m_pN3D_Denoise_toW             ));
    ::memset(m_pN3D_MFHR_Before_Blend,           0, sizeof(m_pN3D_MFHR_Before_Blend       ));
    ::memset(m_pN3D_MFHR_MFHR_Single,            0, sizeof(m_pN3D_MFHR_MFHR_Single        ));
    ::memset(m_pN3D_MFHR_MFB,                    0, sizeof(m_pN3D_MFHR_MFB                ));
    ::memset(m_pN3D_MFHR_After_Blend,            0, sizeof(m_pN3D_MFHR_After_Blend        ));
    ::memset(m_pSWHDR_Phase1,                    0, sizeof(m_pSWHDR_Phase1                ));
    ::memset(m_pSWHDR_Phase2,                    0, sizeof(m_pSWHDR_Phase2                ));
    ::memset(m_pPreview_Preview_ZOOM1,           0, sizeof(m_pPreview_Preview_ZOOM1       ));
    ::memset(m_pPreview_Preview_ZOOM2,           0, sizeof(m_pPreview_Preview_ZOOM2       ));
    ::memset(m_pPreview_Video_ZOOM1,             0, sizeof(m_pPreview_Video_ZOOM1         ));
    ::memset(m_pPreview_Video_ZOOM2,             0, sizeof(m_pPreview_Video_ZOOM2         ));
    ::memset(m_pzHDR_Capture_Flash,              0, sizeof(m_pzHDR_Capture_Flash          ));
    ::memset(m_pAuto_zHDR_Capture_Flash,         0, sizeof(m_pAuto_zHDR_Capture_Flash     ));


    //LINK_SENSORs_SCENEs_ISOs(LINK_CAPTURE);
    //LINK_SENSORs_SCENEs_ISOs(LINK_VIDEO);
    //LINK_SENSORs_SCENEs_ISOs(LINK_PREVIEW);


//==================================================================================================

#define LINK_Preview_Capture_ZOOM1(sensor, scene, iso)\
m_pPreview_Capture_Zoom1[iso] = &IdxSet<EIspProfile_Preview_Capture_ZOOM1, sensor, scene, iso>::idx

#define LINK_Preview_Capture_ZOOM2(sensor, scene, iso)\
m_pPreview_Capture_Zoom2[iso] = &IdxSet<EIspProfile_Preview_Capture_ZOOM2, sensor, scene, iso>::idx

#define LINK_Video_Preview_ZOOM1(sensor, scene, iso)\
m_pVideo_Preview_Zoom1[iso] = &IdxSet<EIspProfile_Video_Preview_ZOOM1, sensor, scene, iso>::idx

#define LINK_Video_Preview_ZOOM2(sensor, scene, iso)\
m_pVideo_Preview_Zoom2[iso] = &IdxSet<EIspProfile_Video_Preview_ZOOM2, sensor, scene, iso>::idx

#define LINK_Video_Video_ZOOM1(sensor, scene, iso)\
m_pVideo_Video_Zoom1[iso] = &IdxSet<EIspProfile_Video_Video_ZOOM1, sensor, scene, iso>::idx

#define LINK_Video_Video_ZOOM2(sensor, scene, iso)\
m_pVideo_Video_Zoom2[iso] = &IdxSet<EIspProfile_Video_Video_ZOOM2, sensor, scene, iso>::idx

#define LINK_Capture_Capture_ZOOM1(sensor, scene, iso)\
m_pCapture_Capture_Zoom1[iso] = &IdxSet<EIspProfile_Capture_Capture_ZOOM1, sensor, scene, iso>::idx

#define LINK_Capture_Capture_ZOOM2(sensor, scene, iso)\
m_pCapture_Capture_Zoom2[iso] = &IdxSet<EIspProfile_Capture_Capture_ZOOM2, sensor, scene, iso>::idx

#define LINK_MFNR_Before_ZOOM0(sensor, scene, iso)\
m_pMFNR_Before_Zoom0[iso] = &IdxSet<EIspProfile_MFNR_Before_ZOOM0, sensor, scene, iso>::idx

#define LINK_MFNR_Before_ZOOM1(sensor, scene, iso)\
m_pMFNR_Before_Zoom1[iso] = &IdxSet<EIspProfile_MFNR_Before_ZOOM1, sensor, scene, iso>::idx

#define LINK_MFNR_Before_ZOOM2(sensor, scene, iso)\
m_pMFNR_Before_Zoom2[iso] = &IdxSet<EIspProfile_MFNR_Before_ZOOM2, sensor, scene, iso>::idx

#define LINK_MFNR_Single_ZOOM0(sensor, scene, iso)\
m_pMFNR_Single_Zoom0[iso] = &IdxSet<EIspProfile_MFNR_Single_ZOOM0, sensor, scene, iso>::idx

#define LINK_MFNR_Single_ZOOM1(sensor, scene, iso)\
m_pMFNR_Single_Zoom1[iso] = &IdxSet<EIspProfile_MFNR_Single_ZOOM1, sensor, scene, iso>::idx

#define LINK_MFNR_Single_ZOOM2(sensor, scene, iso)\
m_pMFNR_Single_Zoom2[iso] = &IdxSet<EIspProfile_MFNR_Single_ZOOM2, sensor, scene, iso>::idx

#define LINK_MFNR_MFB_ZOOM0(sensor, scene, iso)\
m_pMFNR_MFB_Zoom0[iso] = &IdxSet<EIspProfile_MFNR_MFB_ZOOM0, sensor, scene, iso>::idx

#define LINK_MFNR_MFB_ZOOM1(sensor, scene, iso)\
m_pMFNR_MFB_Zoom1[iso] = &IdxSet<EIspProfile_MFNR_MFB_ZOOM1, sensor, scene, iso>::idx

#define LINK_MFNR_MFB_ZOOM2(sensor, scene, iso)\
m_pMFNR_MFB_Zoom2[iso] = &IdxSet<EIspProfile_MFNR_MFB_ZOOM2, sensor, scene, iso>::idx

#define LINK_MFNR_After_ZOOM0(sensor, scene, iso)\
m_pMFNR_After_Zoom0[iso] = &IdxSet<EIspProfile_MFNR_After_ZOOM0, sensor, scene, iso>::idx

#define LINK_MFNR_After_ZOOM1(sensor, scene, iso)\
m_pMFNR_After_Zoom1[iso] = &IdxSet<EIspProfile_MFNR_After_ZOOM1, sensor, scene, iso>::idx

#define LINK_MFNR_After_ZOOM2(sensor, scene, iso)\
m_pMFNR_After_Zoom2[iso] = &IdxSet<EIspProfile_MFNR_After_ZOOM2, sensor, scene, iso>::idx

#define LINK_Flash_Capture_ZOOM1(sensor, scene, iso)\
m_pFlash_Cap_Zoom1[iso] = &IdxSet<EIspProfile_Flash_Capture_ZOOM1, sensor, scene, iso>::idx

#define LINK_Flash_Capture_ZOOM2(sensor, scene, iso)\
m_pFlash_Cap_Zoom2[iso] = &IdxSet<EIspProfile_Flash_Capture_ZOOM2, sensor, scene, iso>::idx

#define LINK_HDR_PREVIEW(sensor, scene, iso)\
m_pHDR_Preview[iso] = &IdxSet<EIspProfile_iHDR_Preview, sensor, scene, iso>::idx

#define LINK_HDR_VIDEO(sensor, scene, iso)\
m_pHDR_Video[iso] = &IdxSet<EIspProfile_iHDR_Video, sensor, scene, iso>::idx

#define LINK_HDR_PREVIEW_VSS(sensor, scene, iso)\
m_pHDR_Preview_VSS[iso] = &IdxSet<EIspProfile_iHDR_Preview_VSS, sensor, scene, iso>::idx

#define LINK_HDR_VIDEO_VSS(sensor, scene, iso)\
m_pHDR_Video_VSS[iso] = &IdxSet<EIspProfile_iHDR_Video_VSS, sensor, scene, iso>::idx

#define LINK_HDR_CAPTURE(sensor, scene, iso)\
m_pHDR_Capture[iso] = &IdxSet<EIspProfile_zHDR_Capture, sensor, scene, iso>::idx

#define LINK_AUTO_HDR_PREVIEW(sensor, scene, iso)\
m_pAuto_HDR_Preview[iso] = &IdxSet<EIspProfile_Auto_iHDR_Preview, sensor, scene, iso>::idx

#define LINK_AUTO_HDR_VIDEO(sensor, scene, iso)\
m_pAuto_HDR_Video[iso] = &IdxSet<EIspProfile_Auto_iHDR_Video, sensor, scene, iso>::idx

#define LINK_AUTO_HDR_PREVIEW_VSS(sensor, scene, iso)\
m_pAuto_HDR_Preview_VSS[iso] = &IdxSet<EIspProfile_Auto_iHDR_Preview_VSS, sensor, scene, iso>::idx

#define LINK_AUTO_HDR_VIDEO_VSS(sensor, scene, iso)\
m_pAuto_HDR_Video_VSS[iso] = &IdxSet<EIspProfile_Auto_iHDR_Video_VSS, sensor, scene, iso>::idx

#define LINK_AUTO_HDR_CAPTURE(sensor, scene, iso)\
m_pAuto_HDR_Capture[iso] = &IdxSet<EIspProfile_Auto_zHDR_Capture, sensor, scene, iso>::idx

#define LINK_MFNR_BEFORE(sensor, scene, iso)\
m_pMFNR_Before[iso] = &IdxSet<EIspProfile_MFNR_Before_Blend, sensor, scene, iso>::idx

#define LINK_MFNR_SINGLE(sensor, scene, iso)\
m_pMFNR_Single[iso] = &IdxSet<EIspProfile_MFNR_Single, sensor, scene, iso>::idx

#define LINK_MFNR_MFB(sensor, scene, iso)\
m_pMFNR_MFB[iso] = &IdxSet<EIspProfile_MFNR_MFB, sensor, scene, iso>::idx

#define LINK_MFNR_AFTER(sensor, scene, iso)\
m_pMFNR_After[iso] = &IdxSet<EIspProfile_MFNR_After_Blend, sensor, scene, iso>::idx

#define LINK_ZHDR_MFNR_BEFORE(sensor, scene, iso)\
m_pMFNR_Before_zCap[iso] = &IdxSet<EIspProfile_zHDR_Capture_MFNR_Before_Blend, sensor, scene, iso>::idx

#define LINK_ZHDR_MFNR_SINGLE(sensor, scene, iso)\
m_pMFNR_Single_zCap[iso] = &IdxSet<EIspProfile_zHDR_Capture_MFNR_Single, sensor, scene, iso>::idx

#define LINK_ZHDR_MFNR_MFB(sensor, scene, iso)\
m_pMFNR_MFB_zCap[iso] = &IdxSet<EIspProfile_zHDR_Capture_MFNR_MFB, sensor, scene, iso>::idx

#define LINK_ZHDR_MFNR_AFTER(sensor, scene, iso)\
m_pMFNR_After_zCap[iso] = &IdxSet<EIspProfile_zHDR_Capture_MFNR_After_Blend, sensor, scene, iso>::idx

#define LINK_EIS_PREVIEW(sensor, scene, iso)\
m_pEIS_Preview[iso] = &IdxSet<EIspProfile_EIS_Preview, sensor, scene, iso>::idx

#define LINK_EIS_VIDEO(sensor, scene, iso)\
m_pEIS_Video[iso] = &IdxSet<EIspProfile_EIS_Video, sensor, scene, iso>::idx

#define LINK_HDR_EIS_PREVIEW(sensor, scene, iso)\
m_pEIS_HDR_Preview[iso] = &IdxSet<EIspProfile_EIS_iHDR_Preview, sensor, scene, iso>::idx

#define LINK_HDR_EIS_VIDEO(sensor, scene, iso)\
m_pEIS_HDR_Video[iso] = &IdxSet<EIspProfile_EIS_iHDR_Video, sensor, scene, iso>::idx

#define LINK_AUTO_HDR_EIS_PREVIEW(sensor, scene, iso)\
m_pEIS_Auto_HDR_Preview[iso] = &IdxSet<EIspProfile_EIS_Auto_iHDR_Preview, sensor, scene, iso>::idx

#define LINK_AUTO_HDR_EIS_VIDEO(sensor, scene, iso)\
m_pEIS_Auto_HDR_Video[iso] = &IdxSet<EIspProfile_EIS_Auto_iHDR_Video, sensor, scene, iso>::idx

#define LINK_MNR(sensor, scene, iso)\
m_pMNR[iso] = &IdxSet<EIspProfile_Capture_MultiPass_HWNR, sensor, scene, iso>::idx

#define LINK_YUV_REPROCESS(sensor, scene, iso)\
m_pYUV_Reprocess[iso] = &IdxSet<EIspProfile_YUV_Reprocess, sensor, scene, iso>::idx

#define LINK_FLASH_CAPTURE(sensor, scene, iso)\
m_pFlash_Cap[iso] = &IdxSet<EIspProfile_Flash_Capture, sensor, scene, iso>::idx

#define LINK_N3D_PREVIEW(sensor, scene, iso)\
m_pN3D_Preview[iso] = &IdxSet<EIspProfile_N3D_Preview, sensor, scene, iso>::idx

#define LINK_N3D_VIDEO(sensor, scene, iso)\
m_pN3D_Video[iso] = &IdxSet<EIspProfile_N3D_Video, sensor, scene, iso>::idx

#define LINK_N3D_CAPTURE(sensor, scene, iso)\
m_pN3D_Capture[iso] = &IdxSet<EIspProfile_N3D_Capture, sensor, scene, iso>::idx

#define LINK_N3D_DENOISE(sensor, scene, iso)\
m_pN3D_Denoise[iso] = &IdxSet<EIspProfile_N3D_Denoise, sensor, scene, iso>::idx

#define LINK_N3D_DENOISE_TOGGM(sensor, scene, iso)\
m_pN3D_Denoise_toGGM[iso] = &IdxSet<EIspProfile_N3D_Denoise_toGGM, sensor, scene, iso>::idx

#define LINK_N3D_DENOISE_TOYUV(sensor, scene, iso)\
m_pN3D_Denoise_toYUV[iso] = &IdxSet<EIspProfile_N3D_Denoise_toYUV, sensor, scene, iso>::idx

#define LINK_N3D_DENOISE_TOW(sensor, scene, iso)\
m_pN3D_Denoise_toW[iso] = &IdxSet<EIspProfile_N3D_Denoise_toW, sensor, scene, iso>::idx

#define LINK_N3D_MFHR_BEFORE_BLEND(sensor, scene, iso)\
m_pN3D_MFHR_Before_Blend[iso] = &IdxSet<EIspProfile_N3D_MFHR_Before_Blend, sensor, scene, iso>::idx

#define LINK_N3D_MFHR_SINGLE(sensor, scene, iso)\
m_pN3D_MFHR_MFHR_Single[iso] = &IdxSet<EIspProfile_N3D_MFHR_Single, sensor, scene, iso>::idx

#define LINK_N3D_MFHR_MFB(sensor, scene, iso)\
m_pN3D_MFHR_MFB[iso] = &IdxSet<EIspProfile_N3D_MFHR_MFB, sensor, scene, iso>::idx

#define LINK_N3D_MFHR_BEFORE_AFTER_BLEND(sensor, scene, iso)\
m_pN3D_MFHR_After_Blend[iso] = &IdxSet<EIspProfile_N3D_MFHR_After_Blend, sensor, scene, iso>::idx

#define LINK_SWHDR_PHASE1(sensor, scene, iso)\
m_pSWHDR_Phase1[iso] = &IdxSet<EIspProfile_SWHDR_Phase1, sensor, scene, iso>::idx

#define LINK_SWHDR_PHASE2(sensor, scene, iso)\
m_pSWHDR_Phase2[iso]= &IdxSet<EIspProfile_SWHDR_Phase2, sensor, scene, iso>::idx

#define LINK_PREVIEW_PREVIEW_ZOOM1(sensor, scene, iso)\
m_pPreview_Preview_ZOOM1[iso]= &IdxSet<EIspProfile_Preview_Preview_ZOOM1, sensor, scene, iso>::idx

#define LINK_PREVIEW_PREVIEW_ZOOM2(sensor, scene, iso)\
m_pPreview_Preview_ZOOM2[iso]= &IdxSet<EIspProfile_Preview_Preview_ZOOM2, sensor, scene, iso>::idx

#define LINK_PREVIEW_VIDEO_ZOOM1(sensor, scene, iso)\
m_pPreview_Video_ZOOM1[iso]= &IdxSet<EIspProfile_Preview_Video_ZOOM1, sensor, scene, iso>::idx

#define LINK_PREVIEW_VIDEO_ZOOM2(sensor, scene, iso)\
m_pPreview_Video_ZOOM2[iso]= &IdxSet<EIspProfile_Preview_Video_ZOOM2, sensor, scene, iso>::idx

#define LINK_HDR_CAPTURE_FLASH(sensor, scene, iso)\
m_pzHDR_Capture_Flash[iso]= &IdxSet<EIspProfile_zHDR_Flash_Capture, sensor, scene, iso>::idx

#define LINK_AUTO_HDR_CAPTURE_FLASH(sensor, scene, iso)\
m_pAuto_zHDR_Capture_Flash[iso]= &IdxSet<EIspProfile_Auto_zHDR_Flash_Capture, sensor, scene, iso>::idx

LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_HDR_PREVIEW, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_HDR_VIDEO, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_HDR_PREVIEW_VSS, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_HDR_VIDEO_VSS, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_HDR_CAPTURE, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_AUTO_HDR_PREVIEW, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_AUTO_HDR_VIDEO, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_AUTO_HDR_PREVIEW_VSS, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_AUTO_HDR_VIDEO_VSS, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_AUTO_HDR_CAPTURE, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_MFNR_BEFORE, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_MFNR_SINGLE, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_MFNR_MFB, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_MFNR_AFTER, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_ZHDR_MFNR_BEFORE, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_ZHDR_MFNR_SINGLE, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_ZHDR_MFNR_MFB, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_ZHDR_MFNR_AFTER, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_EIS_PREVIEW, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_EIS_VIDEO, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_HDR_EIS_PREVIEW, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_HDR_EIS_VIDEO, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_AUTO_HDR_EIS_PREVIEW, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_AUTO_HDR_EIS_VIDEO, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_MNR, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_YUV_REPROCESS, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_FLASH_CAPTURE, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_N3D_PREVIEW, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_N3D_VIDEO, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_N3D_CAPTURE, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_N3D_DENOISE, 0, 0);

LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_N3D_DENOISE_TOGGM, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_N3D_DENOISE_TOYUV, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_N3D_DENOISE_TOW, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_N3D_MFHR_BEFORE_BLEND, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_N3D_MFHR_SINGLE, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_N3D_MFHR_MFB, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_N3D_MFHR_BEFORE_AFTER_BLEND, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_SWHDR_PHASE1, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_SWHDR_PHASE2, 0, 0);

LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_Preview_Capture_ZOOM1, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_Preview_Capture_ZOOM2, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_Video_Preview_ZOOM1, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_Video_Preview_ZOOM2, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_Video_Video_ZOOM1, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_Video_Video_ZOOM2, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_Capture_Capture_ZOOM1, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_Capture_Capture_ZOOM2, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_MFNR_Before_ZOOM0, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_MFNR_Before_ZOOM1, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_MFNR_Before_ZOOM2, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_MFNR_Single_ZOOM0, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_MFNR_Single_ZOOM1, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_MFNR_Single_ZOOM2, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_MFNR_MFB_ZOOM0, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_MFNR_MFB_ZOOM1, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_MFNR_MFB_ZOOM2, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_MFNR_After_ZOOM0, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_MFNR_After_ZOOM1, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_MFNR_After_ZOOM2, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_Flash_Capture_ZOOM1, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_Flash_Capture_ZOOM2, 0, 0);

LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_PREVIEW_PREVIEW_ZOOM1, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_PREVIEW_PREVIEW_ZOOM2, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_PREVIEW_VIDEO_ZOOM1, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_PREVIEW_VIDEO_ZOOM2, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_HDR_CAPTURE_FLASH, 0, 0);
LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_AUTO_HDR_CAPTURE_FLASH, 0, 0);

//==================================================================================================

/*
#define LINK_MFB_CAPTURE_EE_OFF(sensor, scene, iso)\
m_pMFB_Capture_EE_Off[sensor][iso] = &IdxSet<EIspProfile_MFB_Capture_EE_Off, sensor, scene, iso>::idx

    LINK_ONE_SCENE_SENSORS_ISOs(LINK_MFB_CAPTURE_EE_OFF, 0);
*/

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
INDEX_T const*
IdxSetMgr::
get(MUINT32 ispProfile, MUINT32 sensor/*=0*/, MUINT32 const scene_/*=0*/, MUINT32 const iso/*=0*/, MUINT32 const zoom) const
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


    MUINT32 scene = MTK_CONTROL_SCENE_MODE_UNSUPPORTED;

#if SCENE_MODE_MAPPING_EN
    scene = scene_;
#endif

    switch  (ispProfile)
    {
    //  Normal
    case EIspProfile_Preview:
        if (sensor == ESensorMode_Capture)
        {
           if ((zoom == eIDX_ZOOM_0) || ((zoom == eIDX_ZOOM_OFF)))
               return  get_Preview(sensor, scene, iso);
           else if (zoom == eIDX_ZOOM_1)
               return  get_Preview_Capture_Zoom1(sensor, scene, iso);
           else
               return  get_Preview_Capture_Zoom2(sensor, scene, iso);
        }
        else if (sensor == ESensorMode_Preview)
        {
           if ((zoom == eIDX_ZOOM_0) || ((zoom == eIDX_ZOOM_OFF)))
               return  get_Preview(sensor, scene, iso);
           else if (zoom == eIDX_ZOOM_1)
               return  get_Preview_Preview_ZOOM1(sensor, scene, iso);
           else
               return  get_Preview_Preview_ZOOM2(sensor, scene, iso);
        }
        else if (sensor == ESensorMode_Video)
        {
           if ((zoom == eIDX_ZOOM_0) || ((zoom == eIDX_ZOOM_OFF)))
               return  get_Preview(sensor, scene, iso);
           else if (zoom == eIDX_ZOOM_1)
               return  get_Preview_Video_ZOOM1(sensor, scene, iso);
           else
               return  get_Preview_Video_ZOOM2(sensor, scene, iso);
        }
        else return  get_Preview(sensor, scene, iso);
    case EIspProfile_Video:
        if (sensor == ESensorMode_Preview)
        {
           if ((zoom == eIDX_ZOOM_0) || ((zoom == eIDX_ZOOM_OFF)))
               return  get_Video(sensor, scene, iso);
           else if (zoom == eIDX_ZOOM_1)
               return  get_Video_Preview_Zoom1(sensor, scene, iso);
           else
               return  get_Video_Preview_Zoom2(sensor, scene, iso);
        }
        else if (sensor == ESensorMode_Video)
        {
           if ((zoom == eIDX_ZOOM_0) || ((zoom == eIDX_ZOOM_OFF)))
               return  get_Video(sensor, scene, iso);
           else if (zoom == eIDX_ZOOM_1)
               return  get_Video_Video_Zoom1(sensor, scene, iso);
           else
               return  get_Video_Video_Zoom2(sensor, scene, iso);
        }
        else return  get_Video(sensor, scene, iso);
    case EIspProfile_Capture:
        if (sensor == ESensorMode_Capture)
        {
           if ((zoom == eIDX_ZOOM_0) || ((zoom == eIDX_ZOOM_OFF)))
               return  get_Capture(sensor, scene, iso);
           else if (zoom == eIDX_ZOOM_1)
               return  get_Capture_Capture_Zoom1(sensor, scene, iso);
           else
               return  get_Capture_Capture_Zoom2(sensor, scene, iso);
        }
        else return  get_Capture(sensor, scene, iso);
    case EIspProfile_iHDR_Preview:
    case EIspProfile_zHDR_Preview:
    case EIspProfile_mHDR_Preview:
        return  get_HDR_Preview(sensor, scene, iso);
    case EIspProfile_iHDR_Video:
    case EIspProfile_zHDR_Video:
    case EIspProfile_mHDR_Video:
        return  get_HDR_Video(sensor, scene, iso);
    case EIspProfile_iHDR_Preview_VSS:
    case EIspProfile_zHDR_Preview_VSS:
    case EIspProfile_mHDR_Preview_VSS:
        return  get_HDR_Preview_VSS(sensor, scene, iso);
    case EIspProfile_iHDR_Video_VSS:
    case EIspProfile_zHDR_Video_VSS:
    case EIspProfile_mHDR_Video_VSS:
        return  get_HDR_Video_VSS(sensor, scene, iso);
    case EIspProfile_zHDR_Capture:
    case EIspProfile_mHDR_Capture:
        return  get_HDR_Capture(sensor, scene, iso);
    case EIspProfile_Auto_iHDR_Preview:
    case EIspProfile_Auto_zHDR_Preview:
    case EIspProfile_Auto_mHDR_Preview:
        return  get_Auto_HDR_Preview(sensor, scene, iso);
    case EIspProfile_Auto_iHDR_Video:
    case EIspProfile_Auto_zHDR_Video:
    case EIspProfile_Auto_mHDR_Video:
        return  get_Auto_HDR_Video(sensor, scene, iso);
    case EIspProfile_Auto_iHDR_Preview_VSS:
    case EIspProfile_Auto_zHDR_Preview_VSS:
    case EIspProfile_Auto_mHDR_Preview_VSS:
        return  get_Auto_HDR_Preview_VSS(sensor, scene, iso);
    case EIspProfile_Auto_iHDR_Video_VSS:
    case EIspProfile_Auto_zHDR_Video_VSS:
    case EIspProfile_Auto_mHDR_Video_VSS:
        return  get_Auto_HDR_Video_VSS(sensor, scene, iso);
    case EIspProfile_Auto_zHDR_Capture:
    case EIspProfile_Auto_mHDR_Capture:
        return  get_Auto_HDR_Capture(sensor, scene, iso);
    case EIspProfile_MFNR_Before_Blend:
        {
            if (zoom == eIDX_ZOOM_OFF)
                return  get_MFNR_Before_Blend(sensor, scene, iso);
            if (zoom == eIDX_ZOOM_0)
                return  get_MFNR_Before_Blend_Zoom0(sensor, scene, iso);
            else if (zoom == eIDX_ZOOM_1)
                return  get_MFNR_Before_Blend_Zoom1(sensor, scene, iso);
            else
                return  get_MFNR_Before_Blend_Zoom2(sensor, scene, iso);
        }
    case EIspProfile_MFNR_Single:
        {
            if (zoom == eIDX_ZOOM_OFF)
                return  get_MFNR_Single(sensor, scene, iso);
            if (zoom == eIDX_ZOOM_0)
                return  get_MFNR_Single_Zoom0(sensor, scene, iso);
            else if (zoom == eIDX_ZOOM_1)
                return  get_MFNR_Single_Zoom1(sensor, scene, iso);
            else
                return  get_MFNR_Single_Zoom2(sensor, scene, iso);
        }
    case EIspProfile_MFNR_MFB:
        {
            if (zoom == eIDX_ZOOM_OFF)
                return  get_MFNR_MFB(sensor, scene, iso);
            if (zoom == eIDX_ZOOM_0)
                return  get_MFNR_MFB_Zoom0(sensor, scene, iso);
            else if (zoom == eIDX_ZOOM_1)
                return  get_MFNR_MFB_Zoom1(sensor, scene, iso);
            else
                return  get_MFNR_MFB_Zoom2(sensor, scene, iso);
        }
    case EIspProfile_MFNR_After_Blend:
        {
            if (zoom == eIDX_ZOOM_OFF)
                return  get_MFNR_After_Blend(sensor, scene, iso);
            if (zoom == eIDX_ZOOM_0)
                return  get_MFNR_After_Blend_Zoom0(sensor, scene, iso);
            else if (zoom == eIDX_ZOOM_1)
                return  get_MFNR_After_Blend_Zoom1(sensor, scene, iso);
            else
                return  get_MFNR_After_Blend_Zoom2(sensor, scene, iso);
        }
    case EIspProfile_zHDR_Capture_MFNR_Before_Blend:
        return  get_zCapture_MFNR_Before_Blend(sensor, scene, iso);
    case EIspProfile_zHDR_Capture_MFNR_Single:
        return  get_zCapture_MFNR_Single(sensor, scene, iso);
    case EIspProfile_zHDR_Capture_MFNR_MFB:
        return  get_zCapture_MFNR_MFB(sensor, scene, iso);
    case EIspProfile_zHDR_Capture_MFNR_After_Blend:
        return  get_zCapture_MFNR_After_Blend(sensor, scene, iso);
    case EIspProfile_EIS_Preview:
        return  get_EIS_Preview(sensor, scene, iso);
    case EIspProfile_EIS_Video:
        return  get_EIS_Video(sensor, scene, iso);
    case EIspProfile_EIS_iHDR_Preview:
    case EIspProfile_EIS_zHDR_Preview:
    case EIspProfile_EIS_mHDR_Preview:
        return  get_EIS_HDR_Preview(sensor, scene, iso);
    case EIspProfile_EIS_iHDR_Video:
    case EIspProfile_EIS_zHDR_Video:
    case EIspProfile_EIS_mHDR_Video:
        return  get_EIS_HDR_Video(sensor, scene, iso);
    case EIspProfile_EIS_Auto_iHDR_Preview:
    case EIspProfile_EIS_Auto_zHDR_Preview:
    case EIspProfile_EIS_Auto_mHDR_Preview:
        return  get_EIS_Auto_HDR_Preview(sensor, scene, iso);
    case EIspProfile_EIS_Auto_iHDR_Video:
    case EIspProfile_EIS_Auto_zHDR_Video:
    case EIspProfile_EIS_Auto_mHDR_Video:
        return  get_EIS_Auto_HDR_Video(sensor, scene, iso);
    case EIspProfile_Capture_MultiPass_HWNR:
        return  get_Capture_MultiPass_HWNR(sensor, scene, iso);
    case EIspProfile_YUV_Reprocess:
        return get_YUV_Reprocess(sensor, scene, iso);
    case EIspProfile_Flash_Capture:
        {
            if (zoom == eIDX_ZOOM_1)
                return  get_Flash_Capture_Zoom1(sensor, scene, iso);
            else if (zoom == eIDX_ZOOM_2)
                return  get_Flash_Capture_Zoom2(sensor, scene, iso);
            else
                return  get_Flash_Capture(sensor, scene, iso);
        }
    case EIspProfile_N3D_Preview:
    case EIspProfile_N3D_Preview_toW:
        return  get_N3D_Preview(sensor, scene, iso);
    case EIspProfile_N3D_Video:
    case EIspProfile_N3D_Video_toW:
        return  get_N3D_Video(sensor, scene, iso);
    case EIspProfile_N3D_Capture:
    case EIspProfile_N3D_Capture_toW:
        return  get_N3D_Capture(sensor, scene, iso);
    case EIspProfile_N3D_Denoise_toGGM:
        return get_N3D_Denoise_toGGM(sensor, scene, iso);
    case EIspProfile_N3D_Denoise_toYUV:
        return get_N3D_Denoise_toYUV(sensor, scene, iso);
    case EIspProfile_N3D_Denoise_toW:
        return get_N3D_Denoise_toW(sensor, scene, iso);
    case EIspProfile_N3D_Denoise:
        return  get_N3D_Denoise(sensor, scene, iso);
    case EIspProfile_N3D_MFHR_Before_Blend:
        return get_N3D_MFHR_Before_Blend(sensor, scene, iso);
    case EIspProfile_N3D_MFHR_Single:
        return get_N3D_MFHR_Single(sensor, scene, iso);
    case EIspProfile_N3D_MFHR_MFB:
        return get_N3D_MFHR_MFB(sensor, scene, iso);
    case EIspProfile_N3D_MFHR_After_Blend:
        return get_N3D_MFHR_After_Blend(sensor, scene, iso);
    case EIspProfile_SWHDR_Phase1:
        return get_SWHDR_Phase1(sensor, scene, iso);
    case EIspProfile_SWHDR_Phase2:
        return get_SWHDR_Phase2(sensor, scene, iso);
    case EIspProfile_zHDR_Flash_Capture:
    case EIspProfile_mHDR_Flash_Capture:
        return get_zHDR_Capture_Flash(sensor, scene, iso);
    case EIspProfile_Auto_zHDR_Flash_Capture:
    case EIspProfile_Auto_mHDR_Flash_Capture:
        return get_Auto_zHDR_Capture_Flash(sensor, scene, iso);
    default:
        break;
    }
    return  NULL;
}


