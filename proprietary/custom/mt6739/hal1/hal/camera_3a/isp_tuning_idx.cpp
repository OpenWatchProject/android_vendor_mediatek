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
#include "cfg_isp_tuning_idx_preview.h"
#include "cfg_isp_tuning_idx_video.h"
#include "cfg_isp_tuning_idx_capture.h"
#include "cfg_isp_tuning_idx_feature.h"

using namespace NSIspTuning;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
namespace
{
    enum EIndexNum
    {
        NUM_OBC         =   NVRAM_OBC_TBL_NUM,
        NUM_DBS         =   NVRAM_DBS_TBL_NUM,
        NUM_BPC         =   NVRAM_BPC_TBL_NUM,
        NUM_NR1         =   NVRAM_NR1_TBL_NUM,
        NUM_PDC         =   NVRAM_PDC_TBL_NUM,
        NUM_CFA         =   NVRAM_CFA_TBL_NUM,
        NUM_GGM         =   NVRAM_GGM_TBL_NUM,
        NUM_ANR         =   NVRAM_ANR_TBL_NUM,
        NUM_CCR         =   NVRAM_CCR_TBL_NUM,
        NUM_EE          =   NVRAM_EE_TBL_NUM
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
MBOOL IndexMgr::setIdx_CFA(UINT16 const idx)   { return setIdx<NUM_CFA>(CFA, idx); }
//MBOOL IndexMgr::setIdx_GGM(UINT16 const idx)   { return setIdx<NUM_GGM>(GGM, idx); }
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
        " OBC:%d, BPC:%d, NR1:%d, PDC:%d, CFA:%d, GGM:%d, ANR:%d, CCR:%d, EE:%d"
        , OBC, BNR_BPC, BNR_NR1, BNR_PDC, CFA, ANR, CCR, EE
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
            r.linkIndexSet();
        }
    } link_singleton(singleton);

    return  singleton;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IdxSetMgr::
linkIndexSet()
{
    ::memset(m_pPreview,              0, sizeof(m_pPreview));
    ::memset(m_pVideo,                0, sizeof(m_pVideo));
    ::memset(m_pCapture,              0, sizeof(m_pCapture));
    ::memset(m_pMulti_Pass_ANR1,      0, sizeof(m_pMulti_Pass_ANR1));
    ::memset(m_pMulti_Pass_ANR2,      0, sizeof(m_pMulti_Pass_ANR2));
    ::memset(m_pFlash_Cap,            0, sizeof(m_pFlash_Cap));

#define LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, scene)\
    link(sensor, scene, eIDX_ISO_100);\
    link(sensor, scene, eIDX_ISO_200);\
    link(sensor, scene, eIDX_ISO_400);\
    link(sensor, scene, eIDX_ISO_800);\
    link(sensor, scene, eIDX_ISO_1200);\
    link(sensor, scene, eIDX_ISO_1600);\
    link(sensor, scene, eIDX_ISO_2000);\
    link(sensor, scene, eIDX_ISO_2400);\
    link(sensor, scene, eIDX_ISO_2800);\
    link(sensor, scene, eIDX_ISO_3200);\
    link(sensor, scene, eIDX_ISO_4000);\
    link(sensor, scene, eIDX_ISO_4800);\
    link(sensor, scene, eIDX_ISO_5600);\
    link(sensor, scene, eIDX_ISO_6400);\
    link(sensor, scene, eIDX_ISO_7200);\
    link(sensor, scene, eIDX_ISO_8000);\
    link(sensor, scene, eIDX_ISO_8800);\
    link(sensor, scene, eIDX_ISO_9600);\
    link(sensor, scene, eIDX_ISO_11200);\
    link(sensor, scene, eIDX_ISO_12800);



#define LINK_ONE_SCENE_SENSORS_ISOs(link, scene)\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, ESensorMode_Preview, scene);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, ESensorMode_Video, scene);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, ESensorMode_Capture, scene);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, ESensorMode_SlimVideo1, scene);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, ESensorMode_SlimVideo2, scene);

#define LINK_ONE_SENSOR_SCENEs_ISOs(link, sensor)\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_UNSUPPORTED);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_NORMAL);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_ACTION);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_PORTRAIT);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_LANDSCAPE);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_NIGHT);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_NIGHT_PORTRAIT);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_THEATRE);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_BEACH);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_SNOW);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_SUNSET);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_STEADYPHOTO);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_FIREWORKS);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_SPORTS);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_PARTY);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_CANDLELIGHT);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_HDR);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_FACE_PRIORITY);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_BARCODE);

#define LINK_SENSORs_SCENEs_ISOs(link)\
    LINK_ONE_SENSOR_SCENEs_ISOs(link, ESensorMode_Preview);\
    LINK_ONE_SENSOR_SCENEs_ISOs(link, ESensorMode_Video);\
    LINK_ONE_SENSOR_SCENEs_ISOs(link, ESensorMode_Capture);\
    LINK_ONE_SENSOR_SCENEs_ISOs(link, ESensorMode_SlimVideo1);\
    LINK_ONE_SENSOR_SCENEs_ISOs(link, ESensorMode_SlimVideo2);\
    LINK_ONE_SENSOR_SCENEs_ISOs(link, ESensorMode_Custom1);\
    LINK_ONE_SENSOR_SCENEs_ISOs(link, ESensorMode_Custom2);\
    LINK_ONE_SENSOR_SCENEs_ISOs(link, ESensorMode_Custom3);\
    LINK_ONE_SENSOR_SCENEs_ISOs(link, ESensorMode_Custom4);\
    LINK_ONE_SENSOR_SCENEs_ISOs(link, ESensorMode_Custom5);

#if SCENE_MODE_MAPPING_EN

#define LINK_CAPTURE(sensor, scene, iso)\
m_pCapture[sensor][scene][iso] = &IdxSet<EIspProfile_Capture, sensor, scene, iso>::idx

#define LINK_VIDEO(sensor, scene, iso)\
m_pVideo[sensor][scene][iso] = &IdxSet<EIspProfile_Video, sensor, scene, iso>::idx

#define LINK_PREVIEW(sensor, scene, iso)\
m_pPreview[sensor][scene][iso] = &IdxSet<EIspProfile_Preview, sensor, scene, iso>::idx

#else

#define LINK_CAPTURE(sensor, scene, iso)\
    m_pCapture[sensor][scene][iso] = &IdxSet<EIspProfile_Capture, sensor, MTK_CONTROL_SCENE_MODE_UNSUPPORTED, iso>::idx

#define LINK_VIDEO(sensor, scene, iso)\
    m_pVideo[sensor][scene][iso] = &IdxSet<EIspProfile_Video, sensor, MTK_CONTROL_SCENE_MODE_UNSUPPORTED, iso>::idx

#define LINK_PREVIEW(sensor, scene, iso)\
    m_pPreview[sensor][scene][iso] = &IdxSet<EIspProfile_Preview, sensor, MTK_CONTROL_SCENE_MODE_UNSUPPORTED, iso>::idx

#endif

    LINK_SENSORs_SCENEs_ISOs(LINK_CAPTURE);
    LINK_SENSORs_SCENEs_ISOs(LINK_VIDEO);
    LINK_SENSORs_SCENEs_ISOs(LINK_PREVIEW);


//==================================================================================================
#if SCENE_MODE_MAPPING_EN

#define LINK_MULTI_PASS_ANR1(sensor, scene, iso)\
m_pMulti_Pass_ANR1[iso] = &IdxSet<EIspProfile_Capture_MultiPass_ANR_1, sensor, scene, iso>::idx

#define LINK_MULTI_PASS_ANR2(sensor, scene, iso)\
m_pMulti_Pass_ANR2[iso] = &IdxSet<EIspProfile_Capture_MultiPass_ANR_2, sensor, scene, iso>::idx

#define LINK_FLASH_CAPTURE(sensor, scene, iso)\
m_pFlash_Cap[iso] = &IdxSet<EIspProfile_FLASH_Capture, sensor, scene, iso>::idx

#else

#define LINK_MULTI_PASS_ANR1(sensor, scene, iso)\
m_pMulti_Pass_ANR1[iso] = &IdxSet<EIspProfile_Capture_MultiPass_ANR_1, sensor, MTK_CONTROL_SCENE_MODE_UNSUPPORTED, iso>::idx

#define LINK_MULTI_PASS_ANR2(sensor, scene, iso)\
m_pMulti_Pass_ANR2[iso] = &IdxSet<EIspProfile_Capture_MultiPass_ANR_2, sensor, MTK_CONTROL_SCENE_MODE_UNSUPPORTED, iso>::idx

#define LINK_FLASH_CAPTURE(sensor, scene, iso)\
m_pFlash_Cap[iso] = &IdxSet<EIspProfile_FLASH_Capture, sensor, MTK_CONTROL_SCENE_MODE_UNSUPPORTED, iso>::idx


#endif

    LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_MULTI_PASS_ANR1, 0, 0);
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_MULTI_PASS_ANR2, 0, 0);
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(LINK_FLASH_CAPTURE, 0, 0);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
INDEX_T const*
IdxSetMgr::
get(MUINT32 ispProfile, MUINT32 sensor/*=0*/, MUINT32 const /*scene_=0*/, MUINT32 const iso/*=0*/) const
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
        return  get_Preview(sensor, scene, iso);
    case EIspProfile_Video:
    case EIspProfile_VFB_PostProc:
        return  get_Video(sensor, scene, iso);
    case EIspProfile_Capture:
    case EIspProfile_ZSD_Capture:
    case EIspProfile_VSS_Capture:
    case EIspProfile_PureRAW_Capture:
    case EIspProfile_PureRAW_TPipe_Capture:
    case EIspProfile_MFB_PostProc_ANR_EE:
    case EIspProfile_VSS_MFB_PostProc_ANR_EE:
    case EIspProfile_MFB_PostProc_ANR_EE_SWNR:
    case EIspProfile_VSS_MFB_PostProc_ANR_EE_SWNR:
    case EIspProfile_Capture_SWNR:
    case EIspProfile_VSS_Capture_SWNR:
    case EIspProfile_PureRAW_Capture_SWNR:
    case EIspProfile_PureRAW_TPipe_Capture_SWNR:
        return  get_Capture(sensor, scene, iso);
    case EIspProfile_MHDR_Capture: // temp solution: scenario = video, sensor mode = capture
        return  get_Video(sensor, scene, iso);
    case EIspProfile_Capture_MultiPass_ANR_1:
    case EIspProfile_VSS_Capture_MultiPass_ANR_1:
    case EIspProfile_MFB_MultiPass_ANR_1:
    case EIspProfile_VSS_MFB_MultiPass_ANR_1:
        return get_Multi_Pass_ANR1(sensor, scene, iso);
    case EIspProfile_Capture_MultiPass_ANR_2:
    case EIspProfile_VSS_Capture_MultiPass_ANR_2:
    case EIspProfile_MFB_MultiPass_ANR_2:
    case EIspProfile_VSS_MFB_MultiPass_ANR_2:
        return get_Multi_Pass_ANR2(sensor, scene, iso);
    default:
        break;
    }
    return  NULL;
}


