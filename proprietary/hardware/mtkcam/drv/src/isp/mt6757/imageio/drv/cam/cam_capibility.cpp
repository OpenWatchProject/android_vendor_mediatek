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
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "cam_capibitiliy"

#include <cam/cam_capibility.h>

#include <mtkcam/drv/def/ICam_type.h>
#include <mtkcam/drv/def/ispio_port_index.h>
#include <mtkcam/def/ImageFormat.h>

#include <cutils/properties.h>  // For property_get().
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif


#ifndef USING_MTK_LDVT   // Not using LDVT.
#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1
    #include <cutils/log.h>

    #define CAM_CAP_DBG(fmt, arg...)    ALOGD(LOG_TAG"[%s](%5d)"     fmt, __FUNCTION__, __LINE__, ##arg)
    #define CAM_CAP_INF(fmt, arg...)    ALOGD(LOG_TAG"[%s](%5d)"     fmt, __FUNCTION__, __LINE__, ##arg)
    #define CAM_CAP_WRN(fmt, arg...)    ALOGW(LOG_TAG"[%s]WRN(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
    #define CAM_CAP_ERR(fmt, arg...)    ALOGE(LOG_TAG"[%s]ERR(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)

#else   // Using LDVT.
    #include "uvvf.h"

    #define CAM_CAP_DBG(fmt, arg...)    VV_MSG(LOG_TAG"[%s](%5d)"     fmt, __FUNCTION__, __LINE__, ##arg)
    #define CAM_CAP_INF(fmt, arg...)    VV_MSG(LOG_TAG"[%s](%5d)"     fmt, __FUNCTION__, __LINE__, ##arg)
    #define CAM_CAP_WRN(fmt, arg...)    VV_MSG(LOG_TAG"[%s]WRN(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
    #define CAM_CAP_ERR(fmt, arg...)    VV_MSG(LOG_TAG"[%s]ERR(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)

#endif  // USING_MTK_LDVT


MBOOL capibility::GetCapibility(MUINT32 portId,E_CAM_Query e_Op,CAM_Queryrst &QueryRst)
{
    MBOOL ret = MTRUE;

    if(e_Op & ECamio_BS_ratio){
        if( (QueryRst.ratio = this->GetRatio(portId)) == 0 )
            ret = MFALSE;
    }

    if(e_Op & ECamio_valid_fmt){
        if(this->GetFormat(portId, QueryRst) == MFALSE)
            ret = MFALSE;
    }

    if(e_Op & ECamio_pipeline_bitdepth){
        if( (QueryRst.pipelinebitdepth = this->GetPipelineBitdepth()) < 0)
            ret = MFALSE;
    }

    if(e_Op & (~ECamio_valid_cmd)){
        ret = MFALSE;
        CAM_CAP_ERR("some query cmd(0x%x) is not supported. valid cmd(0x%x)\n",e_Op,ECamio_valid_cmd);
    }

    return ret;
}

MUINT32 capibility::GetFormat(MUINT32 portID, CAM_Queryrst &QueryRst)
{
    switch(portID){
        case NSImageio::NSIspio::EPortIndex_IMGO:
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_BAYER8);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_BAYER10);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_BAYER12);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_BAYER10_MIPI);
            break;
        case NSImageio::NSIspio::EPortIndex_RRZO:
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_FG_BAYER8);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_FG_BAYER10);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_FG_BAYER12);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_UFEO_BAYER8);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_UFEO_BAYER10);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_UFEO_BAYER12);
            break;
        default:
            CAM_CAP_ERR("current portID(0x%x) are not supported in query\n",portID);
            return MFALSE;
            break;
    }
    return MTRUE;
}

MUINT32 capibility::GetRatio(MUINT32 portID)
{
    #define MAX_SCALING_DWON_RATIO  (25)        //unit:%

    switch(portID){
        case NSImageio::NSIspio::EPortIndex_RRZO:
            return MAX_SCALING_DWON_RATIO;
            break;
        default:
            CAM_CAP_ERR("curent portID(0x%x) are no scaler\n",portID);
            break;

    }
    return 100;
}

MUINT32 capibility::GetPipelineBitdepth()
{
    return (NSCam::NSIoPipe::NSCamIOPipe::CAM_Pipeline_12BITS);
}


