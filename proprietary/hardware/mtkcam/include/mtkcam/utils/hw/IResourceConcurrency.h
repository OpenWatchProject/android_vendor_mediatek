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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_IRESOURCE_CONCURRENCY_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_IRESOURCE_CONCURRENCY_H_
//
#include <utils/RefBase.h>
#include <mtkcam/def/common.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface of Resource Concurrency
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class IResourceConcurrency
    : public virtual android::RefBase
{
public:
    enum CLIENT_HANDLER
    {
        CLIENT_HANDLER_0    = 0,
        CLIENT_HANDLER_1,
        CLIENT_HANDLER_MAX,                         // the amount of CLIENT_HANDLER
        CLIENT_HANDLER_NULL = CLIENT_HANDLER_MAX    // undefined CLIENT_HANDLER
    };

    /**
     * This utility is for the resource to control the concurrency.
     */

public:
    /**
     * The static function to create the instance of ResourceConcurrency.
     * And this function will return a SP of IResourceConcurrency.
     */
    static android::sp<IResourceConcurrency>
                            createInstance(
                                const char * name,
                                MINT64 timeout_ms);

public:
    /**
     * Request the Client-Handler of this resource.
     * If all Client-Handler are requested, no more available Client-Handler,
     * it will return CLIENT_HANDLER_NULL.
     */
    virtual CLIENT_HANDLER  requestClient() = 0;

    /**
     * Return the Client-Handler of this resource.
     * After this Client-Handler returned, this ID cannot be used anymore.
     */
    virtual MERROR          returnClient(CLIENT_HANDLER id) = 0;

    /**
     * Acquire the resource by Client-Handler.
     */
    virtual MERROR          acquireResource(CLIENT_HANDLER id) = 0;

    /**
     * Release the resource by Client-Handler.
     */
    virtual MERROR          releaseResource(CLIENT_HANDLER id) = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam

#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_IRESOURCE_CONCURRENCY_H_

