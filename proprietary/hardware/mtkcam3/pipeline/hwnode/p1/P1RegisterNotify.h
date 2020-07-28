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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_HWNODE_P1_REGISTER_NOTIFY_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_HWNODE_P1_REGISTER_NOTIFY_H_

#define LOG_TAG "MtkCam/P1NodeImp"
//
#include "P1Common.h"
#include "P1Utility.h"
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::NSP1Node;
using namespace NSCam::Utils::Sync;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NS3Av3;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
namespace NSCam {
namespace v3 {
namespace NSP1Node {


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/
class P1NodeImp;
class P1RegisterNotify;


/******************************************************************************
 *
 ******************************************************************************/
class P1NotifyCrop
    : public P1_TUNING_NOTIFY
{

public:
    P1NotifyCrop(sp<P1RegisterNotify> spP1Notify)
        : mspP1Notify(spP1Notify)
    {};

    virtual ~P1NotifyCrop()
    {};

    virtual const char* TuningName()
    { return "P1NodeCrop"; };

    virtual void p1TuningNotify(MVOID* pIn, MVOID* pOut);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Data Member.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    sp<P1RegisterNotify>    mspP1Notify;

};


/******************************************************************************
 *
 ******************************************************************************/
class P1NotifyQuality
    : public P1_TUNING_NOTIFY
{

public:
    P1NotifyQuality(sp<P1RegisterNotify> spP1Notify)
        : mspP1Notify(spP1Notify)
    {};

    virtual ~P1NotifyQuality()
    {};

    virtual const char* TuningName()
    { return "P1NodeQuality"; };

    virtual void p1TuningNotify(MVOID* pIn, MVOID* pOut);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Data Member.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    sp<P1RegisterNotify>    mspP1Notify;

};


/******************************************************************************
 *
 ******************************************************************************/
class P1NotifyYuvR1Crop
    : public P1_TUNING_NOTIFY
{

public:
    P1NotifyYuvR1Crop(sp<P1RegisterNotify> spP1Notify)
        : mspP1Notify(spP1Notify)
    {};

    virtual ~P1NotifyYuvR1Crop()
    {};

    virtual const char* TuningName()
    { return "P1NodeYuvR1Crop"; };

    virtual void p1TuningNotify(MVOID* pIn, MVOID* pOut);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Data Member.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    sp<P1RegisterNotify>    mspP1Notify;

};

/******************************************************************************
 *
 ******************************************************************************/
class P1NotifyYuvR2Crop
    : public P1_TUNING_NOTIFY
{

public:
    P1NotifyYuvR2Crop(sp<P1RegisterNotify> spP1Notify)
        : mspP1Notify(spP1Notify)
    {};

    virtual ~P1NotifyYuvR2Crop()
    {};

    virtual const char* TuningName()
    { return "P1NodeYuvR2Crop"; };

    virtual void p1TuningNotify(MVOID* pIn, MVOID* pOut);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Data Member.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    sp<P1RegisterNotify>    mspP1Notify;

};



/******************************************************************************
 *
 ******************************************************************************/
class P1RegisterNotify
    : public virtual android::RefBase
{

public:

    P1RegisterNotify(sp<P1NodeImp> spP1NodeImp);

    virtual ~P1RegisterNotify();

public:
    MVOID       init(MVOID);

    MVOID       uninit(MVOID);

    MVOID       config(MVOID);

    MVOID       doNotifyCrop(MVOID * pIn, MVOID * pOut);

    MVOID       doNotifyYuvR1Crop(MVOID * pIn, MVOID * pOut);

    MVOID       doNotifyYuvR2Crop(MVOID * pIn, MVOID * pOut);

    MVOID       doNotifyQuality(MVOID * pIn, MVOID * pOut);

    MVOID *     getNotifyCrop(MVOID);

    MVOID *     getNotifyYuvR1Crop(MVOID);

    MVOID *     getNotifyYuvR2Crop(MVOID);

    MVOID *     getNotifyQuality(MVOID);

//private:


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Data Member.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    sp<P1NodeImp>       mspP1NodeImp;
    MINT32              mOpenId;
    MINT32              mLogLevel;
    MINT32              mLogLevelI;
    MUINT8              mBurstNum;
    //
    P1NotifyCrop *      mpNotifyCrop;
    //
    P1NotifyQuality *   mpNotifyQuality;
    //
    P1NotifyYuvR1Crop *   mpNotifyYuvR1Crop;
    //
    P1NotifyYuvR2Crop *   mpNotifyYuvR2Crop;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Function Member.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


};

};//namespace NSP1Node
};//namespace v3
};//namespace NSCam

#endif //_MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_HWNODE_P1_REGISTER_NOTIFY_H_

