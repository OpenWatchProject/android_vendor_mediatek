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
 * @file PipeBufferPoolMgr.h
 * @brief BufferPoolMgr for ThirdParty ISP pipe
*/

#ifndef _MTK_CAMERA_FEATURE_PIPE_ISP_PIPE_BUFFERPOOL_MGR_H_
#define _MTK_CAMERA_FEATURE_PIPE_ISP_PIPE_BUFFERPOOL_MGR_H_


// Standard C header file

// Android system/core header file
#include <utils/KeyedVector.h>
#include <utils/SortedVector.h>
#include <utils/RefBase.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/def/common.h>
#include <mtkcam/feature/stereo/pipe/IIspPipe.h>
// Module header file
#include <featurePipe/core/include/FatImageBufferPool.h>
#include <featurePipe/core/include/GraphicBufferPool.h>
#include <featurePipe/core/include/ImageBufferPool.h>
#include <featurePipe/vsdof/util/TuningBufferPool.h>
// Local header file

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

class PipeBufferHandler;
/*******************************************************************************
* Global Define
********************************************************************************/
typedef FatImageBufferPool PipeImgBufferPool;
typedef SmartFatImageBuffer SmartPipeImgBuffer;

#define CREATE_IMGBUF_POOL(rBufPool, name, size, format, usage) \
    MY_LOGD("Create PipeImgBufferPool:: "#name " size=%dx%d format=%x usage=%x", \
                size.w, size.h, format, usage); \
    rBufPool = PipeImgBufferPool::create(name, size.w, size.h, format, usage);

#define CREATE_INDIVI_IMGBUF_POOL(rBufPool, name, size, format, usage) \
    MY_LOGD("Create not continuous ImageBufferPool:: "#name " size=%dx%d format=%x usage=%x", \
                size.w, size.h, format, usage); \
    rBufPool = ImageBufferPool::create(name, size.w, size.h, format, usage, MFALSE);

#define CREATE_GRABUF_POOL(rBufPool, name, size, format, usage) \
    MY_LOGD("Create GraphicBufferPool:: "#name " size=%dx%d format=%x usage=%x", \
                size.w, size.h, format, usage); \
    rBufPool = GraphicBufferPool::create(name, size.w, size.h, format, usage);

#define ALLOCATE_BUFFER_POOL(bufPool, size) \
    MY_LOGD("ALLOCATE_BUFFER_POOL: " #bufPool " ---- %d", size);\
    if(!bufPool->allocate(size))\
    { \
        MY_LOGE("Allocate "#bufPool" image buffer failed"); \
    }
/*******************************************************************************
* Enum Define
********************************************************************************/
// Buffer type enum
typedef enum eIspBufferType
{
    eBUFFER_IMAGE,      /*!< indicate the image buffer */
    eBUFFER_GRAPHIC,    /*!< indicate the gralloc buffer */
    eBUFFER_TUNING,     /*!< indicate the tuning buffer */
    // any buffer need to be define above this line.
    eBUFFER_SIZE,
    eBUFFER_INVALID

} IspBufferType;

/**
 * @class PipeBufferPoolMgr
 * @brief buffer pool mgr for isp pipe
 */
class PipeBufferPoolMgr : public virtual android::VirtualLightRefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    PipeBufferPoolMgr(sp<IspPipeSetting> pPipeSetting);
    virtual ~PipeBufferPoolMgr();

    /**
     * @brief get node count
     * @return
     * - node count
     */
    MVOID setNodeCount(MINT32 nodeCount);

    /**
     * @brief get node count
     * @return
     * - node count
     */
    MINT32 getNodeCount() const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PipeBufferPoolMgr Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief add the specific buffer pool to the mgr
     * @param [in] id: buffer pool id.
     * @param [in] pool: the buffer pool that created from outside
     */
    MVOID addPool(
                        IspPipeBufferID id,
                        sp<PipeImgBufferPool> pool);
    /**
     * @brief add the specific non-continuous buffer pool to the mgr
     * @param [in] id: buffer pool id.
     * @param [in] pool: the buffer pool that created from outside
     */
    MVOID addPool(
                        IspPipeBufferID id,
                        sp<ImageBufferPool> pool);
    /**
     * @brief add the specific tuning buffer pool to the mgr
     * @param [in] id: buffer pool id.
     * @param [in] pool: the tuning buffer pool that created from outside
     */
    MVOID addTBPool(
                    IspPipeBufferID id,
                    sp<TuningBufferPool> pool);
    /**
     * @brief request buffer from the pipe bufferPool belong to the buffer id
     * @param [in] id buffer id used to request buffer.
     * @return
     * - requested buffer from buffer pool
     */
    SmartPipeImgBuffer request(
                            IspPipeBufferID id
                            );
    /**
     * @brief request individual(non-continuous) buffer
     *        from the image bufferPool belong to the buffer id
     * @param [in] id buffer id used to request buffer.
     * @return
     * - requested buffer from buffer pool
     */
    SmartImageBuffer requestIB(
                            IspPipeBufferID id
                            );
    /**
     * @brief request graphic buffer from the bufferPool belong to the buffer id
     * @param [in] id buffer id used to request buffer.
     * @return
     * - requested buffer from buffer pool
     */
    SmartGraphicBuffer requestGB(
                            IspPipeBufferID id
                            );

    /**
     * @brief request tuning buffer from the bufferPool belong to the buffer id
     * @param [in] id buffer id used to request buffer.
     * @return
     * - requested buffer from buffer pool
     */
    SmartTuningBuffer requestTB(
                            IspPipeBufferID id
                            );

    /**
     * @brief create the PipeBufferHandler
     * @return
     * - PipeBufferHandler pointer
     */
    sp<PipeBufferHandler> createBufferHandler();

    /**
     * @brief query the buffer type of the working buffer ID
     * @param [in] id buffer id used to query
     * @return
     * - buffer type
     */
    IspBufferType queryBufferType(IspPipeBufferID bid);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PipeBufferPoolMgr Private Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**
     * @brief uninit function
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL uninit();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PipeBufferPoolMgr Public Member.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PipeBufferPoolMgr Private Member.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MINT32 mNodeCount;
    // buffer id to tuning buffer pool map
    KeyedVector<IspPipeBufferID, sp<TuningBufferPool> > mBIDtoTuningBufferPoollMap;
    // buffer id to image buffer pool map
    KeyedVector<IspPipeBufferID, sp<PipeImgBufferPool> > mBIDtoImgBufPoolMap;
    // buffer id to individual(non-continuous) image buffer pool map
    KeyedVector<IspPipeBufferID, sp<ImageBufferPool> > mBIDtoIndividualImgBufPoolMap;
    // buffer id to image type
    KeyedVector<IspPipeBufferID, IspBufferType > mBIDtoBufferTyepMap;
};


}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam


#endif