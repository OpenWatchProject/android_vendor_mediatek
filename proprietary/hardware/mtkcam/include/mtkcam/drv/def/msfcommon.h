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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_MSF_COMMON_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_MSF_COMMON_H_
//
#include <mtkcam/def/common.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>

#ifndef ATRACE_TAG
#define ATRACE_TAG                              ATRACE_TAG_CAMERA
#endif
//
#include <utils/Trace.h>
#define DRV_TRACE_NAME_LENGTH                   32
#define DRV_TRACE_CALL()                        ATRACE_CALL()
#define DRV_TRACE_NAME(name)                    ATRACE_NAME(name)
#define DRV_TRACE_INT(name, value)              ATRACE_INT(name, value)
#define DRV_TRACE_BEGIN(name)                   ATRACE_BEGIN(name)
#define DRV_TRACE_END()                         ATRACE_END()
#define DRV_TRACE_ASYNC_BEGIN(name, cookie)     ATRACE_ASYNC_BEGIN(name, cookie)
#define DRV_TRACE_ASYNC_END(name, cookie)       ATRACE_ASYNC_END(name, cookie)
#define DRV_TRACE_FMT_BEGIN(fmt, arg...)                    \
do{                                                         \
    if( ATRACE_ENABLED() )                                  \
    {                                                       \
        char buf[DRV_TRACE_NAME_LENGTH];                    \
        snprintf(buf, DRV_TRACE_NAME_LENGTH, fmt, ##arg);   \
        DRV_TRACE_BEGIN(buf);                               \
    }                                                       \
}while(0)
#define DRV_TRACE_FMT_END()                     DRV_TRACE_END()



/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
/******************************************************************************
 * YW MSS
 ******************************************************************************/
enum StreamTag
{
    EStreamTag_NORM = 0,
    EStreamTag_VSS = 1,
};

enum MSS_SCENARIO
{
    MSS_P2_DL_MODE = 0,
    MSS_BASE_S2_MODE = 1,
    MSS_BASE_S1_MODE = 2,
    MSS_REFE_S1_MODE = 3,
    MSS_BASE_S1_SMVR_MODE = 4,
    MSS_MODE_NUM,
};

struct MSSConfig
{
    MSS_SCENARIO  mss_scenario;
    enum StreamTag tag;
    unsigned int  scale_Total;

    unsigned int  *omc_tuningBuf;
    IImageBuffer  *mss_mvMap; //omcmv
    IImageBuffer  *mss_omcFrame;
    std::vector<IImageBuffer*>  mss_scaleFrame;
        MSSConfig()
            : mss_scenario(MSS_BASE_S1_MODE)
            , tag(EStreamTag_VSS)
            , scale_Total(0)
            , omc_tuningBuf(0x0)
            , mss_mvMap(0x0)
            , mss_omcFrame(0x0)
            , mss_scaleFrame(0x0)
            {}
};

/******************************************************************************
 * YW MSF
 ******************************************************************************/
enum MSF_SCENARIO
{
    MSF_MFNR_MODE = 0,       //only UT use
    /* TODO: GCE */
    MSF_MFNR_MODE_CAP = 6,   //MFNR standalone

    MSF_MFNR_DL_MODE = 1,
    MSF_MSNR_DL_MODE = 2,
    MSF_DSDN_DL_MODE = 2     //same as 2
};

struct MSFConfig
{
    MSF_SCENARIO  msf_scenario;
    unsigned int  frame_Idx;
    unsigned int  frame_Total;
    unsigned int  scale_Idx;
    unsigned int  scale_Total;

    std::vector<unsigned int*>  msf_tuningBuf;
    std::vector<unsigned int*>  msf_sramTable;
    IImageBuffer  *msf_imageDifference; //idi
    IImageBuffer  *msf_confidenceMap; //confi
    IImageBuffer  *msf_dsFrame; //dsi
    std::vector<IImageBuffer*>  msf_baseFrame; //basei
    std::vector<IImageBuffer*>  msf_refFrame; //refi
    std::vector<IImageBuffer*>  msf_outFrame; //fso
    std::vector<IImageBuffer*>  msf_weightingMap_in; //wei
    std::vector<IImageBuffer*>  msf_weightingMap_out; //weo
    std::vector<IImageBuffer*>  msf_weightingMap_ds; //dsw
        MSFConfig()
            : msf_scenario(MSF_MFNR_MODE)
            , frame_Idx(0)
            , frame_Total(0)
            , scale_Total(0)
            , msf_tuningBuf(0x0)
            , msf_sramTable(0x0)
            , msf_imageDifference(0x0)
            , msf_confidenceMap(0x0)
            , msf_dsFrame(0x0)
            , msf_baseFrame(0x0)
            , msf_refFrame(0x0)
            , msf_outFrame(0x0)
            , msf_weightingMap_in(0x0)
            , msf_weightingMap_out(0x0)
            , msf_weightingMap_ds(0x0)
            {}
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_MSF_COMMON_H_

