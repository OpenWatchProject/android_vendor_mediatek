/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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
#ifndef _MTK_HARDWARE_PIPELINE_EXTENSION_VSDOFCTRLER_H_
#define _MTK_HARDWARE_PIPELINE_EXTENSION_VSDOFCTRLER_H_

#include <utils/Thread.h>
#include <mtkcam/utils/std/StlUtils.h>
#include "BaseVsdofCtrler.h"
#include <DpBlitStream.h>

using namespace android;

namespace NSCam
{
namespace plugin
{

enum
{
    eVSDOF_CTRLER_DO_3RD_PARTY_ALGO        = 0,
    eVSDOF_CTRLER_COPY_MAIN1_TO_OUTPUT        ,
    eVSDOF_CTRLER_COPY_MAIN2_TO_OUTPUT        ,
};

class VsdofCtrler
    : public BaseVsdofCtrler
{
        // this class is not allow to instantiate.
    public:
        VsdofCtrler(MINT32 mode);
        ~VsdofCtrler();
    public:
        using BaseVsdofCtrler::getCaptureNum;
        using BaseVsdofCtrler::getDelayFrameNum;
        using BaseVsdofCtrler::setCaptureNum;
        using BaseVsdofCtrler::setDelayFrameNum;
    //
    // override base operation
    //
    public:
        bool            doDualCamProcess(ReqFrameSP main1Frame,
                                         ReqFrameSP main2Frame) override;
    //
    // thread implementation
    //
    public:
        class VsdofCtrlerThread : public Thread
        {
            public:
                VsdofCtrlerThread()
                    : mMain1FrameForThread(NULL)
                    , mMain2FrameForThread(NULL)
                    , mRectSrc(0,0)
                    , mThreadProcessDone (MTRUE)
                {}
                ~VsdofCtrlerThread() {}
                status_t readyToRun() override;
                bool     threadLoop() override;

                // for thread process
                ReqFrameSP      mMain1FrameForThread;
                ReqFrameSP      mMain2FrameForThread;
                MRect           mRectSrc;

                Mutex           mThreadLock;
                Condition       mThreadCondStart;
                Condition       mThreadCondEnd;
                MBOOL           mThreadProcessDone;
        };

    protected:
        sp<VsdofCtrlerThread>          mVsdofCtrlerThread;

    //
    // implementation
    //
    private:
        IImageBuffer* createEmptyImageBuffer(MSize ImgSize);

        MBOOL doVsdof3rdParty(IImageBuffer* pMain1Img,
                                        IImageBuffer* pMain2Img,
                                        IImageBuffer* pProcessedOutputImg,
                                        IImageBuffer* pDepthmap);
        MBOOL doCopy(MRect rectInput,
                        IImageBuffer* pInputImg,
                        IImageBuffer* pOutputImg,
                        IImageBuffer* pDepthmap );
        MINT32  mMain1OpenId;
        MINT32  mMain2OpenId;
        MUINT32 mVsdofCtrlerCopyTest;
        MUINT32 mVsdofCtrlerDump;
        static MUINT32 mShotCnt;
};
};
};
#endif // _MTK_HARDWARE_PIPELINE_EXTENSION_VSDOFCTRLER_H_
