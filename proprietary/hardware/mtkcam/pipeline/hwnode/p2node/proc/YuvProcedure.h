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



#include "../P2Common.h"
#include "../FrameUtils.h"
#include "../Processor.h"


class YuvProcedure
        : public NSCam::plugin::IVendorManager::IDataCallback
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Processor Creator
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    struct CreateParams {
        MUINT32 uOpenId;
        MBOOL bEnableLog;
    };

    static sp<Processor>    createProcessor(CreateParams &params);

                            YuvProcedure(CreateParams const &params);

    virtual                 ~YuvProcedure() {
                            //CallStack stack;
                            //stack.update();
                            //stack.log("YuvProcedure", ANDROID_LOG_INFO, "dumpCallStack");
                            MY_LOGD_IF(mbEnableLog,"~YuvProcedure");
                            if (mpFrameHandler) {
                                delete mpFrameHandler;
                            }
                        };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Frame Parameter
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

typedef Vector< NSCam::plugin::BufferItem >  BufferItemT;
typedef NSCam::plugin::MetaItem              MetaItemT;
struct  FrameParams {
            MBOOL                               lastCall;
        };
struct  FrameInfo {
            sp<NSCam::plugin::IVendorManager>   pVendor;
            MUINT32                             uFrameNo;
            MUINT32                             uRequestNo;
            sp<MetaHandle>                      outApp;
            sp<MetaHandle>                      outHal;
            MUINT32                             userID;
            BufferItemT                         vInBuff;
            MetaItemT                           meta;
            MBOOL                               needDstBuffer;
            std::map< void*, sp<BufferHandle> > vBufMap;
            //
            void                    dump() const;
        };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Processor Interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static const MBOOL isAsync = MTRUE;

    MERROR onYuvPullParams(
            sp<Request> pRequest,
            FrameParams &params
    );

    MERROR onYuvExecute(
            sp<Request> const pRequest,
            FrameParams const &params
    );

    MERROR onYuvFinish(
            FrameParams const &params,
            MBOOL const success
    );

    MVOID onYuvFlush();

    MVOID onYuvNotify(
            MUINT32 const /*event*/,
            MINTPTR const /*arg1*/,
            MINTPTR const /*arg2*/,
            MINTPTR const /*arg3*/
    ) { };


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IDataCallback Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    virtual status_t            onDataReceived(
                                        MBOOL const isLastCb,
                                        MUINT32 const requestNo,
                                        NSCam::plugin::MetaSet       result,
                                        Vector<NSCam::plugin::BufferItem >   buffers
                                );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Internal Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Procedure Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MBOOL const     mbEnableLog;
    MUINT32 const   muOpenId;
    MUINT32         muDumpBuffer;

    mutable Mutex   mLock;
    FrameHandler<FrameInfo> *                  mpFrameHandler;

};

DECLARE_PROC_TRAIT(Yuv);

