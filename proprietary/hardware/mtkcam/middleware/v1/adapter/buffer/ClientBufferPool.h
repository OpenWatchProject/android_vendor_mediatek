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

#ifndef _MTK_HARDWARE_MTKCAM_V1_ADAPTER_BUFFER_CLIENTBUFFERPOOL_H_
#define _MTK_HARDWARE_MTKCAM_V1_ADAPTER_BUFFER_CLIENTBUFFERPOOL_H_
//
#include <utils/RefBase.h>
#include <utils/Vector.h>
#include <utils/Thread.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProvider.h>
#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include "../inc/ImgBufProvidersManager.h"
#include <RecordBufferSnapShot.h>

/******************************************************************************
 *
 ******************************************************************************/
//
namespace NSCam {
namespace v1 {


class ClientBufferPool
    : public IBufferPool
{
public:
                                        ClientBufferPool(
                                            MINT32  openId,
                                            MBOOL   bUseTransform = MFALSE);

                                        ~ClientBufferPool();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:   //// operations.

    /**
     * Try to acquire a buffer from the pool.
     *
     * @param[in] szCallerName: a null-terminated string for a caller name.
     *
     * @param[out] rpBuffer: a reference to a newly acquired buffer.
     *
     * @return 0 indicates success; non-zero indicates an error code.
     */
    virtual MERROR                      acquireFromPool(
                                            char const*                    szCallerName,
                                            MINT32                         rRequestNo,
                                            android::sp<IImageBufferHeap>& rpBuffer,
                                            MUINT32&                       rTransform
                                        );

    /**
     * Release a buffer to the pool.
     *
     * @param[in] szCallerName: a null-terminated string for a caller name.
     *
     * @param[in] pBuffer: a buffer to release.
     *
     * @return
     *      0 indicates success; non-zero indicates an error code.
     */
    virtual MERROR                      releaseToPool(
                                            char const*                   szCallerName,
                                            MINT32                        rRequestNo,
                                            android::sp<IImageBufferHeap> pBuffer,
                                            MUINT64                       rTimeStamp,
                                            bool                          rErrorResult
                                        );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:    //// debug
    /**
     * Pool name.
     */
    virtual char const*                 poolName() const;

    /**
     * Dump information for debug.
     */
    virtual MVOID                       dumpPool() const;

    /**
     *
     * Set specific camClient as buffer source.
     *
     * @param[in] pSource : source CamClient.
     *
     */
    virtual MERROR                      setCamClient(
                                            char const*                         szCallerName,
                                            android::sp<ImgBufProvidersManager> pSource,
                                            MINT32                              rMode
                                        );

    /**
     *
     * Set specific camClient as buffer source.
     *
     * @param[in] pSource : source CamClient.
     *
     */
    virtual MERROR                      setCamClient(
                                            MINT32                              rMode
                                        );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:   //// acquire buffer
    MERROR                              getBufferFromClient_l(
                                            char const*           szCallerName,
                                            MINT32                rRequestNo,
                                            sp<IImageBufferHeap>& rpBuffer,
                                            MUINT32&              rTransform
                                        );

protected:
    MERROR                              returnBufferToClient_l(
                                            char const*          szCallerName,
                                            MINT32               rRequestNo,
                                            sp<IImageBufferHeap> rpBuffer,
                                            MUINT64              rTimeStamp,
                                            bool                 rErrorResult
                                        );

protected:
    MERROR                              tryGetClient();

public:
    MINT32                              getOpenId() const       { return mOpenId; }
public:
    /**
     *
     * Enable buffer
     *
     * @param[in] en : enable flag
     *
     */
    MBOOL                               enableBuffer(
                                            MBOOL                enable
                                        );



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    void                                onLastStrongRef(const void* /*id*/) {};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    MINT32                              mOpenId;
    MINT32                              mLogLevel;

protected:

    sp<ImgBufProvidersManager>          mpSourceManager;
    KeyedVector< MINT32, sp<IImgBufProvider> >
                                        mvClientMap;

protected:
    struct Buffer_T {
        MINT32               requestNo;
        MBOOL                isReturn;
        MBOOL                error;
        MUINT64              timestamp;
        ImgBufQueNode        node;
        sp<IImgBufProvider>  client;
        sp<IImageBufferHeap> heap;
    };

    mutable Mutex                       mLock;
    KeyedVector< MINT32, Buffer_T >     mClientBufferMap;
    Vector<MINT32>                      mDequeOrderMap;
    MBOOL                               mbUseTransform;
    RecordBufferSnapShot*               mpRecordBufferSnapShot;
    sp<IResourceContainer>              mspResourceContainer;
    MUINT64                             mLastTimestamp;
    MBOOL                               mbEnableBuffer;
    MBOOL                               mbDumpNotReturnBuffer;
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v1
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_V1_ADAPTER_BUFFER_CLIENTBUFFERPOOL_H_

