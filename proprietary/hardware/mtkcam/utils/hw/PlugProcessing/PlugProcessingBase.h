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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#ifndef _MTK_HARDWARE_MTKCAM_UTILS_HW_PLUGPROCESSINGBASE_H_
#define _MTK_HARDWARE_MTKCAM_UTILS_HW_PLUGPROCESSINGBASE_H_
//
#include <dlfcn.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <system/thread_defs.h>
#include <utils/RWLock.h>
#include <utils/Thread.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/std/Trace.h>
//
#include <mtkcam/utils/hw/IPlugProcessing.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Plug Processing.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class PlugProcessingBase
    : public virtual IPlugProcessing
{
public:
                            PlugProcessingBase();
    virtual                 ~PlugProcessingBase();


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPlugProcessing Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MERROR                  init(OP_MODE const opMode);

    MERROR                  uninit(void);

    MERROR                  sendCommand(
                                __attribute__((unused)) MINT32 const cmd,
                                __attribute__((unused)) MINTPTR arg1 = 0,
                                __attribute__((unused)) MINTPTR arg2 = 0,
                                __attribute__((unused)) MINTPTR arg3 = 0,
                                __attribute__((unused)) MINTPTR arg4 = 0);

    MERROR                  waitInitDone(void);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  non-virtual function.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    void                    waitInitExit(void);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////            Operations.
    virtual void            onLastStrongRef(
                                __attribute__((unused)) const void* id);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    class InitThread
        : public android::Thread
    {
    public:
        InitThread(PlugProcessingBase* pPlugProcessing,
            MUINT32 id, MINT32 logLevel)
            : mwpPlugProcessing(pPlugProcessing)
            , mOpenID(id)
            , mLogLevel(logLevel)
        {};

        virtual ~InitThread() {};

        void exit(void);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Thread Interface.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:
        virtual android::status_t   readyToRun();

        virtual bool                threadLoop();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Data Member.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    private:
        android::wp<PlugProcessingBase>
                                    mwpPlugProcessing;
        MUINT32                     mOpenID;
        MINT32                      mLogLevel;
    };


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    class DlFunc
    : public virtual android::RefBase
    {
    public:
        DlFunc(char const * libName, MUINT32 id, MINT32 logLevel)
            : mLock()
            , mOpenID(id)
            , mLogLevel(logLevel)
            , mLibHandle(NULL)

        {
            libOpen(libName);
        };

        virtual ~DlFunc()
        {
            libClose();
        };

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Function Member.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:
        void                    getFunction(char const * funcName,
                                    MVOID ** ppFn);

        MBOOL                   isValid(void);

    private:
        void                    libOpen(char const * libName);

        void                    libClose(void);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Data Member.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    private:
        mutable android::Mutex  mLock;
        MUINT32                 mOpenID;
        MINT32                  mLogLevel;
        MVOID *                 mLibHandle;
    };


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  data member.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    MUINT32                 mOpenID;
    mutable android::Mutex  mLock;
    MINT32                  mLogLevel;
    OP_MODE                 mOpMode;
    mutable android::Mutex  mReadyLock;
    MBOOL                   mInited;
    //
    // for InitThread
    android::sp<InitThread> mspInitThread;
    mutable android::Mutex  mInitLock;
    android::Condition      mInitCond;
    MBOOL                   mInitBegan;
    //
    android::sp<DlFunc>     mspDlFunc;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  virtual function.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    virtual MERROR          initImp(void) { return android::OK; };

    virtual MERROR          uninitImp(void) { return android::OK; };

    virtual MERROR          sendCommandImp(
                                __attribute__((unused)) MINT32 const cmd,
                                __attribute__((unused)) MINTPTR arg1 = 0,
                                __attribute__((unused)) MINTPTR arg2 = 0,
                                __attribute__((unused)) MINTPTR arg3 = 0,
                                __attribute__((unused)) MINTPTR arg4 = 0)
                                { return android::OK; };

};

};  //namespace NSCam

#endif//_MTK_HARDWARE_MTKCAM_UTILS_HW_PLUGPROCESSINGBASE_H_

