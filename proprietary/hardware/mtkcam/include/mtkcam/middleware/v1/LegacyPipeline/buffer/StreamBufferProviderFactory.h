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

#ifndef _MTK_HARDWARE_MTKCAM_BUFFERPROVIDER_STREAMBUFFERPROVIDERFACTORY_H_
#define _MTK_HARDWARE_MTKCAM_BUFFERPROVIDER_STREAMBUFFERPROVIDERFACTORY_H_
//
#include <utils/RefBase.h>
#include <utils/Vector.h>

#include "StreamBufferProvider.h"

using namespace android;
using namespace NSCam::v3;

/******************************************************************************
 *
 ******************************************************************************/
//
namespace NSCam {
namespace v1 {

class StreamBufferProviderFactory
    : public virtual RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static android::sp<StreamBufferProviderFactory> createInstance();

    virtual                            ~StreamBufferProviderFactory() {};

    /**
     *
     * Set image stream info.
     *
     * @param[in] pStreamInfo : ImageStreamInfo.
     *
     */
    virtual MERROR                      setImageStreamInfo(
                                            android::sp<IImageStreamInfo> pStreamInfo
                                        )                                                       = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface. Buffer source.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    /**
     *
     * User provided buffer source.
     * If user does not provide buffer pool, allocate buffer according to stream info.
     *
     * @param[in] pPool : user provided IBufferPool.
     *
     */
    virtual MERROR                      setUsersPool(
                                            sp< IBufferPool > pPool
                                        )                                                       = 0;

    /**
     *
     * User provide rule to pair metadata.
     *
     * @param[in] pPairMeta : user provided rule to pair metadata.
     *
     * @param[in] numberToPair : number of metadata need to be paired with buffer.
     *
     */
    virtual MERROR                      setPairRule(
                                            sp< PairMetadata > pPairMeta,
                                            MINT32             numberToPair
                                        )                                                       = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     *
     * Create StreamBufferProvider.
     * This API must be called after all required information is set.
     *
     * @param[out] pBuffer : provider
     *
     */
    virtual sp<StreamBufferProvider>    create( MBOOL NeedTimeStamp = true)                     = 0;

    /**
     *
     * Create StreamBufferProvider.
     * This API must be called after all required information is set.
     * Will store StreamBufferProvider in consumer container if needed.
     *
     * @param[out] pBuffer : provider
     *
     */
    virtual sp<StreamBufferProvider>    create(
                                            MINT32 rOpenId,
                                            MBOOL  rStoreInContainer,
                                            MBOOL NeedTimeStamp = true
                                        )                                                         = 0;


};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v1
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_V1_BUFFERPROVIDER_STREAMBUFFERPROVIDERFACTORY_H_

