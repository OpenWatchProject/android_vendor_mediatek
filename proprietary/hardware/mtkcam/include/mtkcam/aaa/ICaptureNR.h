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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_AAA_ICAPTURENR_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_AAA_ICAPTURENR_H_

#include <mtkcam/def/common.h>
#include <mtkcam/utils/module/module.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <camera_custom_capture_nr.h>

/******************************************************************************
 *
 ******************************************************************************/
class ISwNR
{
public:
    struct SWNRParam
    {
        MUINT32         iso;
        MBOOL           isMfll;
        MBOOL           isDualCam;
        MINT32          perfLevel;
        //
                        SWNRParam()
                         : iso(0)
                         , isMfll(MFALSE)
                         , isDualCam(MFALSE)
                         , perfLevel(0/*eSWNRPerf_Default*/)
                        {}
    };

public:
    virtual             ~ISwNR() {}

    static ISwNR*       createInstance(MUINT32 const openId);

public:
    virtual MBOOL       doSwNR(SWNRParam const& param, NSCam::IImageBuffer* pBuf) = 0;
    virtual MBOOL       doSwNR(SWNRParam const& param, NSCam::IImageBuffer* pBuf, MINT32 magicNo, NSCam::IMetadata* pInHalMeta = NULL) { return MTRUE;};
    virtual MVOID       init(NSCam::MSize requestSize) {};
    // [in/out] halMetadata
    virtual MBOOL       getDebugInfo(NSCam::IMetadata& halMetadata) const = 0;
    virtual MVOID       dumpParam(char * const filename) = 0;

};


/**
 * @brief The definition of the maker of ISwNR instance.
 */
typedef ISwNR* (*SwNR_FACTORY_T)(MUINT32 const openId);
#define MAKE_SwNR(...) \
    MAKE_MTKCAM_MODULE(MTKCAM_MODULE_ID_AAA_SW_NR, SwNR_FACTORY_T, __VA_ARGS__)


/******************************************************************************
 *
 ******************************************************************************/
#endif  //  _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_AAA_ICAPTURENR_H_
