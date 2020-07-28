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


#define LOG_TAG "rss_hal"

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/def/common.h>
#include <cutils/properties.h>  // For property_get().
#include <rss_hal_imp.h>

#define RSS_DRV_COEFFICIENT_TABLE  (15) //recommeded by HW
#define RSS_DRV_RZ_UINT            (1048576) //2^20

using namespace NSCam::NSIoPipe::NSCamIOPipe;

#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)

#define MY_LOGD_IF(cond, ...)       do { if ( (cond) >= (2) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) >= (1) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) >= (1) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) >= (0) ) { MY_LOGE(__VA_ARGS__); } }while(0)

CAM_ULOG_DECLARE_MODULE_ID(MOD_RSS_HAL);

#define RSS_HAL_BUF_WIDTH   288
#define RSS_HAL_BUF_HEIGHT  512

/*******************************************************************************
*
********************************************************************************/
RSSP1Cb::RSSP1Cb(MVOID *arg)
{
    m_pClassObj = arg;
}

RSSP1Cb::~RSSP1Cb()
{
    m_pClassObj = NULL;
}

void RSSP1Cb::p1TuningNotify(MVOID* pIn, MVOID* pOut)
{
    RssHalImp* pRssHal = reinterpret_cast<RssHalImp*>(m_pClassObj);
    MSize BufSize = pRssHal->GetBufSize();
    MINT32 debugLevel = pRssHal->GetDebugLevel();
    MUINT32 SizeOut_H, SizeOut_V;//H for width, V for height

    RSS_CFG* Output = (RSS_CFG*)pOut;
    RSS_INPUT_INFO* Input = (RSS_INPUT_INFO*)pIn;

    // To get max input w size of RSC
    MUINT32 MaxOutputWidth = (Input->rss_in_w*Input->rss_scale_up_factor/100);// HW limitation
    MUINT32 MaxBufWidth = BufSize.w;
    SizeOut_H = (MaxOutputWidth > MaxBufWidth) ? MaxBufWidth : MaxOutputWidth; //max width size for input of RSC

    // Calculate Outsize.v accroding to input.w to input.h.
    SizeOut_V = (SizeOut_H * Input->rrz_out_h) / (Input->rrz_out_w);

    if(SizeOut_V < 22)
        SizeOut_V = 22;
    else if(SizeOut_V > 511)
        SizeOut_V = 511;

    MY_LOGI_IF(debugLevel, "in(%d,%d) s(%d) out(%d,%d) r(%d,%d) BufSize(%d,%d)", Input->rss_in_w, Input->rss_in_h, Input->rss_scale_up_factor,
        SizeOut_H, SizeOut_V, Input->rrz_out_w, Input->rrz_out_h, BufSize.w, BufSize.h);

    if((SizeOut_H > Input->rss_in_w) || (SizeOut_V > Input->rss_in_h)) {
        MY_LOGE("RSS output size is bigger than input size\n");
        return;
    }

    //Coefficient table 15 is recommended by HW.
    Output->cfg_rss_ctrl_hori_tbl_sel = RSS_DRV_COEFFICIENT_TABLE;
    Output->cfg_rss_ctrl_vert_tbl_sel = RSS_DRV_COEFFICIENT_TABLE;
    Output->cfg_rss_ctrl_hori_en = MTRUE;
    Output->cfg_rss_ctrl_vert_en = MTRUE;

    Output->cfg_rss_out_img = ((SizeOut_V << 16) | (SizeOut_H));
    Output->bypassRSS = MFALSE;
    Output->enRSS = MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
sp<RssHal> RssHal::CreateInstance(char const *userName, MUINT32 sensorIdx)
{
    MY_LOGI("%s sensorIdx %d", userName, sensorIdx);
    return new RssHalImp(sensorIdx);
}


RssHal::~RssHal()
{
}

/*******************************************************************************
*
********************************************************************************/
RssHalImp::RssHalImp(MUINT32 sensorIdx)
    : RssHal()
    , mSensorIdx(sensorIdx)
    , mBufSize(RSS_HAL_BUF_WIDTH, RSS_HAL_BUF_HEIGHT)
    , mpRSSP1Cb(NULL)
    , mDebugLevel(0)
{
    MY_LOGI("sensorIdx(%d)", mSensorIdx);
    mDebugLevel = ::property_get_int32("vendor.debug.camera.rsshal", 0);
    mpRSSP1Cb = new RSSP1Cb(this);
}

RssHalImp::~RssHalImp()
{
    MY_LOGI("sensorIdx(%d)", mSensorIdx);
    if (mpRSSP1Cb)
    {
        delete mpRSSP1Cb;
        mpRSSP1Cb = NULL;
    }
}

MINT32 RssHalImp::Init(INormalPipe *pipe, MSize bufSize)
{
    MY_LOGI("set mBufSize(%d, %d)", bufSize.w, bufSize.h);
    MBOOL ret = MFALSE;
    ret = pipe->sendCommand(ENPipeCmd_SET_RSS_CBFP, (MINTPTR)mpRSSP1Cb, -1, -1);
    if (!ret)
    {
        MY_LOGE("sendCommand ENPipeCmd_SET_RSS_CBFP failed");
        return RSS_HAL_RETURN_INVALID_DRIVER;
    }

    SetBufSize(bufSize);

    return RSS_HAL_RETURN_NO_ERROR;
}

MINT32 RssHalImp::Uninit()
{
    MY_LOGI("mBufSize(%d, %d)", mBufSize.w, mBufSize.h);

    return RSS_HAL_RETURN_NO_ERROR;
}
