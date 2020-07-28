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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef _MTKCAM_HWNODE_P2_BASIC_PROCESSOR_H_
#define _MTKCAM_HWNODE_P2_BASIC_PROCESSOR_H_

#include <mtkcam3/feature/utils/p2/DIPStream.h>

#include "P2_Processor.h"
#include "P2_Util.h"
#include "P2_MDPProcessor.h"

namespace P2
{

class BasicProcessor : virtual public Processor<P2InitParam, P2ConfigParam, sp<P2Request>>
{
public:
    BasicProcessor();
    virtual ~BasicProcessor();

protected:
    virtual MBOOL onInit(const P2InitParam &param);
    virtual MVOID onUninit();
    virtual MVOID onThreadStart();
    virtual MVOID onThreadStop();
    virtual MBOOL onConfig(const P2ConfigParam &param);
    virtual MBOOL onEnque(const sp<P2Request> &request);
    virtual MVOID onNotifyFlush();
    virtual MVOID onWaitFlush();

private:
    class P2Payload;
    class P2Cookie;

private:
    MBOOL initNormalStream();
    MVOID uninitNormalStream();
    MBOOL init3A();
    MVOID uninit3A();
    MVOID onP2CB(const DIPParams &dipParams, sp<P2Payload> &payload);
    MBOOL processVenc(const sp<P2Request> &request);
    MBOOL configVencStream(MBOOL enable, MINT32 fps = 0, MSize size = MSize());

private:
    ILog mLog;
    P2Info mP2Info;
    DIPStream *mDIPStream = NULL;
    IHal3A *mHal3A = NULL;
    NS3Av3::IHalISP *mHalISP = NULL;
    MDPProcessor mMDPProcessor;
    MBOOL mEnableVencStream = MFALSE;
    MUINT32 mExpectMS = 20;
    MUINT32 mCounter = 0;

private:
    class MyTimer
    {
    public:
        MyTimer();
        MyTimer(const ILog &log, MUINT32 index);
        MVOID print() const;
        MVOID setFPS(double fps);
        double getFPS() const;
    public:
        using DebugTimer = NSCam::Utils::DebugTimer;

        ADD_DEBUGTIMER(Total);
        ADD_DEBUGTIMER(Enque);
        ADD_DEBUGTIMER(SetIsp);
        ADD_DEBUGTIMER(Deque);
        ADD_DEBUGTIMER(P2);
        ADD_DEBUGTIMER(Helper);
    private:
        ILog mLog;
        MUINT32 mIndex = 0;
        double mFPS = 0;
    };

    class P2Payload : virtual public android::RefBase
    {
    public:
        P2Payload();
        P2Payload(const sp<P2Request> &request, MUINT32 counter);
        virtual ~P2Payload();
        sp<P2Request> mRequest;
        DIPParams mDIPParams;
        MyTimer mTimer;
        TuningParam mTuning;
        P2Util::SimpleIO mIO;
        P2Obj mP2Obj;
    };

    class P2Cookie
    {
    public:
        P2Cookie(BasicProcessor *parent, const sp<P2Payload> &payload);
        P2Cookie(BasicProcessor *parent, const std::vector<sp<P2Payload>> &payloads);
        ~P2Cookie();
        MVOID updateResult(MBOOL result);
        ILog getILog();

        BasicProcessor *mParent;
        std::vector<sp<P2Payload>> mPayloads;
    };
    android::Mutex mP2CookieMutex;
    android::Condition mP2Condition;
    std::list<P2Cookie*> mP2CookieList;

    enum { NO_CHECK_ORDER = 0, CHECK_ORDER = 1 };

    ILog getILog(const sp<P2Payload> &payload);
    ILog getILog(const std::vector<sp<P2Payload>> &payloads);

    template <typename T>
    MBOOL processP2(T payload);
    DIPParams prepareEnqueDIPParams(sp<P2Payload> payload);
    DIPParams prepareEnqueDIPParams(std::vector<sp<P2Payload>> payload);
    MVOID updateResult(sp<P2Payload> payload, MBOOL result);
    MVOID updateResult(std::vector<sp<P2Payload>> payload, MBOOL result);
    MVOID processP2CB(const DIPParams &dipParams, P2Cookie *cookie);
    template <typename T>
    P2Cookie* createCookie(const ILog &log, const T &payload);
    MBOOL freeCookie(P2Cookie *cookie, MBOOL checkOrder);
    static MVOID p2DIPCB(DIPParams &dipParams);
    MVOID waitP2CBDone();

private:
    std::vector<sp<P2Payload>> mBurstQueue;

    MBOOL processBurst(const sp<P2Payload> &payload);
    MBOOL checkBurst();
    NSCam::Utils::FPSCounter mFPSCounter;
    MUINT8 prevReqtSmvrFps = MTK_SMVR_FPS_30;
};

} // namespace P2

#endif // _MTKCAM_HWNODE_P2_BASIC_PROCESSOR_H_
