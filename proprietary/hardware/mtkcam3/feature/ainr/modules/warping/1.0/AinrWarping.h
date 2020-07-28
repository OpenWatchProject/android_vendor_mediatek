/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
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
#ifndef __AINRWPE_H__
#define __AINRWPE_H__

// AINR Core Lib
#include <mtkcam3/feature/ainr/IAinrWarping.h>
// Pass2 header
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/drv/def/IPostProcFeFm.h>
#include <mtkcam/drv/def/Dip_Notify_datatype.h> //_SRZ_SIZE_INFO_, EDipModule_SRZ1

// STL
#include <memory> //std::shared_ptr
#include <condition_variable>
#include <thread>
#include <future>

// AOSP
#include <utils/RefBase.h>

// Driver header
// Warping driver header
#include <mtkcam/drv/iopipe/PostProc/IWpeStream.h>
#include <mtkcam/drv/iopipe/PostProc/WpeUtility.h>

using android::sp;


// namespace for pass2
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NSCam::NSIoPipe::NSWpe;

namespace ainr {

class AinrWarping : public IAinrWarping {
public:
    enum {
        WARPING_ODD = 0,
        WARPING_EVEN = 1,
    };

public:
    AinrWarping();

/* implementation from IAinrWarping */
public:
    /* Feed config data into warping.
     *  @param params               There're different NVRAM settings in the
     *                              different params.
     */
    enum AinrErr init(IAinrWarping::ConfigParams const& params);


    /* Returns work succed or not.
     *  @param [out] BufferPackage     Represents buffer size. This argument can be
     *                              null.
     *  @return                     Address of the buffer, if read fail returns
     *                              null.
     */
    enum AinrErr doWarping(IAinrWarping::WarpingPackage &infoPack);
    virtual ~AinrWarping(void);
private:
    void bufferDump(IMetadata *halMeta, IImageBuffer* buff, int index, int ispProfle, const char* fileType);
    enum AinrErr checkPackage(const IAinrWarping::WarpingPackage &infoPack);
    enum AinrErr prepareWarping(QParams &qParams, IAinrWarping::WarpingPackage &infoPack, int mode
                                        , WPEAINRParams &wpe_AINRparams, WPEQParams &enqueWpeParams);
private:
    // Sensor id
    int m_sensorId;
    // Normal stream
    std::unique_ptr< INormalStream, std::function<void(INormalStream*)> >
                                    m_pNormalStream;
    // Tuning size
    size_t m_regTableSize;
};
}; /* namespace ainr */
#endif//__AINRWPE_H__