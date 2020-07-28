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

#ifndef _MTK_CAMERA_FEATURE_PIPE_DUALCAM_MF_TP_PIPE_H_
#define _MTK_CAMERA_FEATURE_PIPE_DUALCAM_MF_TP_PIPE_H_

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <featurePipe/core/include/CamPipe.h>
#include <mtkcam/feature/stereo/pipe/IDualCamMultiFramePipe.h>
#include <featurePipe/vsdof/util/P2Operator.h>

// Local header file
#include "DualCamMFPipeNode.h"
#include "nodes/BssNode.h"
#include "nodes/PreProcessNode.h"
#include "nodes/VendorNode.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSDCMF {

using namespace android;
/*******************************************************************************
* Enum Define
********************************************************************************/




/*******************************************************************************
* Structure Define
********************************************************************************/




/*******************************************************************************
* Class Define
********************************************************************************/
class DualCamMFPipe
    : public CamPipe<DualCamMFPipeNode>
    , public DualCamMFPipeNode::Handler_T
    , public IDualCamMFPipe
    , public Thread
{
typedef CamPipe<DualCamMFPipeNode> PARENT;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
// Constructor
DualCamMFPipe(MINT32 openSensorIndex);
// Copy constructor
// Create instance
// Destr instance

// destructor
virtual ~DualCamMFPipe();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief initalization
     * @return
     *-true indicates success, otherwise indicates fail
     */
    MBOOL init();
    MBOOL uninit();
    MBOOL enque(PipeRequestPtr& request);
    MVOID flush();
    MVOID setFlushOnStop(MBOOL flushOnStop) { PARENT::setFlushOnStop(flushOnStop); }
    MVOID sync();
    DualCamMFType getPipeFeatureType() { return DualCamMFType::TYPE_DCMF_3rdParty; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Protected Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MBOOL onInit();
    virtual MVOID onUninit();

    virtual MBOOL onData(DataID id, PipeRequestPtr &data);

            MVOID setupP2Operator();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    // thread interface
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    virtual void        requestExit();

    // Good place to do one-time initializations
    virtual status_t    readyToRun();

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool        threadLoop();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Public Data Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Protected Data Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Private Data Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:

MUINT32     miOpenId = 0;
MBOOL       mbDebugLog = MFALSE;
MBOOL       mbProfileLog = MFALSE;

// mutex for request map
android::Mutex mReqMapLock;
// PipeRequest collections
KeyedVector<MUINT32, PipeRequestPtr> mvRequestPtrMap;

BssNode                     mBssNode;
PreProcessNode              mPreProcessNode;
VendorNode                  mVendorNode;

Vector<DualCamMFPipeNode*>  mvBufferProducerNodes;

sp<P2Operator>              mspP2Op = nullptr;

mutable Mutex               mThreadLock;
MBOOL                       mbThreadExit = MFALSE;

KeyedVector<MUINT32, PipeRequestPtr> mvMainRequestPtrMap;
MINT32                               mAccumulatedReqCnt = 0;
MINT32                               mAccumulatedP2DoneCnt = 0;

DefaultKeyedVector<MUINT32, chrono::time_point<chrono::system_clock> > mvCaptureRequestsTime;

MBOOL                       mbQuickPostview = MFALSE;
RequestDumpHelper           mRequestDumpHelper;
};

/*******************************************************************************
* Namespace end.
********************************************************************************/
};
};
};
};

#endif