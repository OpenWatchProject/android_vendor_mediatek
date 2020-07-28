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
    MVOID onP2CB(const QParams &qparams, sp<P2Payload> &payload);
    MVOID process3A(sp<P2Payload> &payload, P2MetaSet &metaSet);
    MVOID prepareQParams(sp<P2Payload> &payload);
    MBOOL processVenc(const sp<P2Request> &request);
    MBOOL configVencStream(MBOOL enable, MINT32 fps = 0, MSize size = MSize());

private:
    sp<P2Info> mP2Info;
    P2ConfigParam mConfigParam;
    INormalStream *mNormalStream;
    IHal3A_T *mHal3A;
    MUINT32 mTuningSize;;
    MDPProcessor mMDPProcessor;
    MBOOL mEnableVencStream;

private:
    class P2Payload : virtual public android::RefBase
    {
    public:
        P2Payload();
        P2Payload(const sp<P2Request> &request);
        virtual ~P2Payload();
        sp<P2Request> mRequest;
        QParams mQParams;
        TuningParam mTuning;
        P2Util::SimpleIO mIO;
    };

    class P2Cookie
    {
    public:
        P2Cookie(BasicProcessor *parent, const sp<P2Payload> &payload);
        P2Cookie(BasicProcessor *parent, const std::vector<sp<P2Payload>> &payloads);
        MVOID updateResult(MBOOL result);
        Logger getLogger();

        BasicProcessor *mParent;
        std::vector<sp<P2Payload>> mPayloads;
    };
    android::Mutex mP2CookieMutex;
    android::Condition mP2Condition;
    std::list<P2Cookie*> mP2CookieList;

    enum { NO_CHECK_ORDER = 0, CHECK_ORDER = 1 };

    Logger getLogger(const sp<P2Payload> &payload);
    Logger getLogger(const std::vector<sp<P2Payload>> &payloads);

    template <typename T>
    MBOOL processP2(T payload);
    QParams prepareEnqueQParams(sp<P2Payload> payload);
    QParams prepareEnqueQParams(std::vector<sp<P2Payload>> payload);
    MVOID updateResult(sp<P2Payload> payload, MBOOL result);
    MVOID updateResult(std::vector<sp<P2Payload>> payload, MBOOL result);
    MVOID processP2CB(const QParams &qparams, P2Cookie *cookie);
    template <typename T>
    P2Cookie* createCookie(const T &payload, const Logger &logger);
    MBOOL freeCookie(P2Cookie *cookie, MBOOL checkOrder);
    static MVOID p2CB(QParams &qparams);
    MVOID waitP2CBDone();

private:
    std::vector<sp<P2Payload>> mBurstQueue;

    MBOOL processBurst(sp<P2Payload> payload);
    MBOOL checkBurst();
};

} // namespace P2

#endif // _MTKCAM_HWNODE_P2_BASIC_PROCESSOR_H_
