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

#ifndef __NORMAL_STREAM_UPPER_H__
#define __NORMAL_STREAM_UPPER_H__


#include <deque>
#include <utils/RefBase.h>
#include <utils/Mutex.h>
#include <utils/Thread.h>
#include <utils/Condition.h>
#include <mtkcam/def/common.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>


namespace NSCam {
namespace NSIoPipe {
namespace NSPostProc {
namespace Wrapper {


class IpRawController : public android::RefBase
{
public:
    static android::sp<IpRawController> getInstance();
    bool acquireP1();
    void asyncReleaseP1();

private:
    class AsyncThread : public android::Thread {
    public:
        AsyncThread(IpRawController &controller) :
            mController(controller)
        {
        }

        virtual bool threadLoop() override;

    private:
        IpRawController &mController;
    };
    friend class AsyncThread;

    static android::Mutex sInstanceMutex;
    static android::wp<IpRawController> sInstance;
    android::Mutex mStateMutex;
    bool mP1IsOccupied;
    int mNumOfP1Acquired;
    android::sp<AsyncThread> mAsyncThread;
    android::Condition mCond;

    IpRawController() :
        mP1IsOccupied(false), mNumOfP1Acquired(0)
    {
    }

    ~IpRawController();

    void init();
    void sendReleasedEvent();
};



class NormalStreamUpper : public INormalStream
{
public:
    // createNormalStream() should create a INormalStream instance and return its pointer.
    // NormalStreamUpper will own the instance and destroy when destructs
    template <typename _F>
    static NormalStreamUpper *createInstance(MUINT32 openedSensorIndex, DipUserParam UserParam, _F &&createNormalStream) {
        return new NormalStreamUpper(openedSensorIndex, createNormalStream(openedSensorIndex, UserParam));
    }

    virtual MVOID destroyInstance();

    virtual MBOOL enque(QParams const& rParams) override;
    virtual MBOOL deque(QParams& rParams, MINT64 i8TimeoutNs) override;

    enum {
        DEBUG_FUNC_LIFE = 0x1
    };

protected:

    virtual ~NormalStreamUpper();

private:

    struct UserData {
        MVOID *mMyCookie;
        QParams::PFN_CALLBACK_T         mpfnCallback;   //deque call back
        QParams::PFN_CALLBACK_T         mpfnEnQFailCallback;    //call back for enque fail
        QParams::PFN_CALLBACK_T         mpfnEnQBlockCallback;   //call back for enque blocking
        MVOID*                          mpCookie;

        UserData() = default;
        UserData(const UserData &userData) = default;
        UserData& operator=(const UserData &userData) = default;
        UserData(MVOID *myCookie, const QParams &qParams);
    };

    typedef unsigned int DebugFlag;

    android::Mutex mMutex;
    std::deque<UserData> mUserData;
    DebugFlag mDebugFlag;
    static constexpr int NUM_OF_COOKIES = 20;
    NormalStreamUpper *mCookiePool[NUM_OF_COOKIES];
    std::deque<MVOID*> mFreeCookies;
    android::sp<IpRawController> mpIpRawController;
    MUINT32 mOpenedSensorIndex;
    INormalStream *mNormalStream;

    NormalStreamUpper(MUINT32 openedSensorIndex, INormalStream *normalStream);

    static bool isIpRawRequest(QParams const& rParams);
    static void markIpRawRequest(QParams& wrpParams);

    bool isMyCookieValid(MVOID *cookie);
    MVOID *allocateCookie();
    void freeCookie(MVOID *cookie);

    template <typename _GetCallback>
    static MVOID transCallback(QParams& rParams, _GetCallback &&getCallback);

    static MVOID enQCallback(QParams& rParams);
    static MVOID enQFailCallback(QParams& rParams);
    static MVOID enQBlockCallback(QParams& rParams);
    bool popUserData(MVOID *myCookie, UserData &userData);
    android::sp<IpRawController> getIpRawController();

public:
    // Following are only forwards
    virtual MBOOL setJpegParam(EJpgCmd jpgCmd,int arg1,int arg2);
    virtual MBOOL setFps(MINT32 fps);
    virtual MBOOL sendCommand(ESDCmd cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3);
    virtual MBOOL init(char const* szCallerName, MUINT32 secTag=0);
    virtual MBOOL init(char const* szCallerName, NSCam::NSIoPipe::EStreamPipeID mPipeID, MUINT32 secTag=0);
    virtual MBOOL uninit(char const* szCallerName);
    virtual MBOOL uninit(char const* szCallerName, NSCam::NSIoPipe::EStreamPipeID mPipeID);
    virtual MERROR getLastErrorCode() const;
    virtual MBOOL startVideoRecord(MINT32 wd,MINT32 ht, MINT32 fps);
    virtual MBOOL stopVideoRecord();
};


}
}
}
}

#endif

