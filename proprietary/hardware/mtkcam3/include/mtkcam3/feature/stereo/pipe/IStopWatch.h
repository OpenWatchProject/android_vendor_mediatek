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

/**
 * @file PipeBufferHandler.h
 * @brief Buffer handler for ispPipe
*/

#ifndef _MTK_CAMERA_FEATURE_PIPE_ISP_PIPE_STOPWATCH_H_
#define _MTK_CAMERA_FEATURE_PIPE_ISP_PIPE_STOPWATCH_H_

// Standard C header file
#include <string>
// Android system/core header file
#include <utils/RefBase.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/def/BuiltinTypes.h>
// Module header file

// Local header file

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

/*******************************************************************************
* Global Define
********************************************************************************/

/*******************************************************************************
* Structure Define
********************************************************************************/

/*******************************************************************************
* Class Definition
********************************************************************************/
enum class StopWatchType
{
    Manual,
    Auto
};

/**
 * @class IStopWatch
 * @brief interface for measurement of the performance
 */
class IStopWatch
    : public android::RefBase
{
public:
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    IStopWatch& operator= (const IStopWatch& other) = delete;
    IStopWatch& operator= (IStopWatch&& other)      = delete;
    IStopWatch(const IStopWatch& other)             = delete;
    IStopWatch(IStopWatch&& other)                  = delete;

    virtual                     ~IStopWatch(){};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IStopWatch Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual const std::string&  getName() const     = 0;
    virtual MVOID               begin()             = 0;
    virtual MVOID               end()               = 0;
    virtual MFLOAT              getElapsed() const  = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IStopWatch Protected Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    IStopWatch() {};
};

/**
 * @class IStopWatchCollection
 * @brief collection class of IStopWatch, that manage IStopWatch objects
 */
class IStopWatchCollection
    : public android::RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IStopWatchCollection Static Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static android::sp<IStopWatchCollection> createInstance(const std::string& name, MUINT32 requestID);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    IStopWatchCollection& operator= (const IStopWatchCollection& other) = delete;
    IStopWatchCollection& operator= (IStopWatchCollection&& other)      = delete;
    IStopWatchCollection(const IStopWatchCollection& other)             = delete;
    IStopWatchCollection(IStopWatchCollection&& other)                  = delete;

    virtual ~IStopWatchCollection(){};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IStopWatchCollection Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual const std::string&          getName() const                                                     = 0;
    virtual android::sp<IStopWatch>     getStopWatch(StopWatchType stopWatchType, const std::string& name)  = 0;
    virtual MVOID                       begin(const std::string& name, MVOID* cookie)                       = 0;
    virtual MVOID                       end(MVOID* cookie)                                                  = 0;
    virtual MVOID                       printResult() const                                                 = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IStopWatchCollection Protected Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    IStopWatchCollection() {};
};



}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam

#endif