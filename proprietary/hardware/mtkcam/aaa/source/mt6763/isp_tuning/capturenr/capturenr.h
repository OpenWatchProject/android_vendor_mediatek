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

#ifndef _MTK_AAA_INC_CAPTRUENR_H_
#define _MTK_AAA_INC_CAPTRUENR_H_

#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/aaa/ICaptureNR.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <MTKNR.h>

class SwNR : public ISwNR
{
public:

                        SwNR(MUINT32 const openId);
                        ~SwNR();

public:
    virtual MBOOL       doSwNR(SWNRParam const& param, NSCam::IImageBuffer* pBuf);
    virtual MBOOL       getDebugInfo(NSCam::IMetadata& halMetadata) const;
    virtual MVOID       dumpParam(char * const filename);

    struct TUNINGINFO
    {
        CCRTuningInfo CCR;
        HFGTuningInfo HFG;

        union uNR
        {
            NR2TuningInfo   NR2;

            uNR() { ::memset(this, 0x00, sizeof(union uNR));}
        } u;

        TUNINGINFO()
        {
            ::memset(&CCR, 0, sizeof(CCR));
            ::memset(&HFG, 0, sizeof(HFG));
        };
    };

private:
    MVOID               enablePerformanceService(MINT32& handle_perf_serv_iir, MINT32 const perfLevel);
    MVOID               disablePerformanceService(MINT32 const handle_perf_serv_iir);
    MUINT32 const       muOpenId;
    DrvNRObject_e       mNRType;
    TUNINGINFO          mTuningInfo;
    sp<NSCam::IImageBuffer>    mpWb;
    MINT32              mDbgReuseWb;
};


#endif  //  _MTK_AAA_INC_CAPTRUENR_H_
